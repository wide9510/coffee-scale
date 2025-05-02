/*
   * NAU7802 Driver for Zephyr RTOS
   *
   * This driver provides an interface to the NAU7802 ADC over I2C.
   *
*/
#include "nau7802.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#define I2C_NODE DT_NODELABEL(i2c0)
#define I2C_ADDR 0x2A
static const struct device *i2c_dev = DEVICE_DT_GET(I2C_NODE);

//y = mx+b
int32_t _zeroOffset = 0;        //This is b
float _calibrationFactor = 1.0; //This is m. User provides this number so that we can output y when requested

unsigned long _ldoRampDelay = 250; //During begin, wait this many millis after configuring the LDO before performing calibrateAFE

static bool getRegister(uint8_t registerAddress, uint8_t *value)
{
  return i2c_write_read(i2c_dev, I2C_ADDR, &registerAddress, 1, value, 1) == 0;
}
static bool setRegister(uint8_t registerAddress, uint8_t value)
{
   uint8_t data[2] = {registerAddress, value};
   return i2c_write(i2c_dev, data, sizeof(data), I2C_ADDR) == 0;
}

//Sets up the NAU7802 for basic function
//If initialize is true (or not specified), default init and calibration is performed
//If initialize is false, then it's up to the caller to initalize and calibrate
//Returns true upon completion
bool begin(bool initialize)
{
   // TODO: remove
   k_busy_wait(1000); //Wait for the I2C bus to stabilize

  //Check if the device ack's over I2C
  if (isConnected() == false)
  {
    //There are rare times when the sensor is occupied and doesn't ack. A 2nd try resolves this.
    if (isConnected() == false)
      return (false);
  }

  bool result = true; //Accumulate a result as we do the setup

  if (initialize)
  {
    result &= reset(); //Reset all registers

    result &= powerUp(); //Power on analog and digital sections of the scale

    result &= setLDO(NAU7802_LDO_3V3); //Set LDO to 3.3V

    result &= setGain(NAU7802_GAIN_128); //Set gain to 128

    result &= setSampleRate(NAU7802_SPS_80); //Set samples per second to 80

    //Turn off CLK_CHP. From 9.1 power on sequencing.
    uint8_t adc;
    getRegister(NAU7802_ADC, &adc);
    adc |= 0x30;
    result &= setRegister(NAU7802_ADC, adc);

    result &= setBit(NAU7802_PGA_PWR_PGA_CAP_EN, NAU7802_PGA_PWR); //Enable 330pF decoupling cap on chan 2. From 9.14 application circuit note.

    result &= clearBit(NAU7802_PGA_LDOMODE, NAU7802_PGA); //Ensure LDOMODE bit is clear - improved accuracy and higher DC gain, with ESR < 1 ohm

    // TODO: remove busy wait
    k_busy_wait(_ldoRampDelay * 1000); //Wait for LDO to stabilize - takes about 200ms

    getWeight(true, 10, 1000); //Flush

    result &= calibrateAFE(NAU7802_CALMOD_INTERNAL); //Re-cal analog front end when we change gain, sample rate, or channel
  }

  return (result);
}

bool isConnected(void) {
   uint8_t dummy = 0x00;
   int ret = i2c_write(i2c_dev, &dummy, 0, I2C_ADDR);  // Send zero-length write
   return (ret == 0);
}

//Returns true if Cycle Ready bit is set (conversion is complete)
bool available()
{
  return (getBit(NAU7802_PU_CTRL_CR, NAU7802_PU_CTRL));
}

//Calibrate analog front end of system. Returns true if CAL_ERR bit is 0 (no error)
//Takes approximately 344ms to calibrate; wait up to 1000ms.
//It is recommended that the AFE be re-calibrated any time the gain, SPS, or channel number is changed.
bool calibrateAFE(NAU7802_Cal_Mode mode)
{
  beginCalibrateAFE(mode);
  return waitForCalibrateAFE(1000);
}

//Begin asynchronous calibration of the analog front end.
// Poll for completion with calAFEStatus() or wait with waitForCalibrateAFE()
void beginCalibrateAFE(NAU7802_Cal_Mode mode)
{
  uint8_t value;
  getRegister(NAU7802_CTRL2, &value);
  value &= 0xFC; // Clear CALMOD bits
  uint8_t calMode = (uint8_t)mode;
  calMode &= 0x03; // Limit mode to 2 bits
  value |= calMode; // Set the mode
  setRegister(NAU7802_CTRL2, value);

  setBit(NAU7802_CTRL2_CALS, NAU7802_CTRL2);
}

//Check calibration status.
NAU7802_Cal_Status calAFEStatus()
{
  if (getBit(NAU7802_CTRL2_CALS, NAU7802_CTRL2))
  {
    return NAU7802_CAL_IN_PROGRESS;
  }

  if (getBit(NAU7802_CTRL2_CAL_ERROR, NAU7802_CTRL2))
  {
    return NAU7802_CAL_FAILURE;
  }

  // Calibration passed
  return NAU7802_CAL_SUCCESS;
}

//Wait for asynchronous AFE calibration to complete with optional timeout.
//If timeout is not specified (or set to 0), then wait indefinitely.
//Returns true if calibration completes succsfully, otherwise returns false.
bool waitForCalibrateAFE(unsigned long timeout_ms)
{
  unsigned long startTime = k_uptime_get_32();
  NAU7802_Cal_Status cal_ready;

  while ((cal_ready = calAFEStatus()) == NAU7802_CAL_IN_PROGRESS)
  {
    if ((timeout_ms > 0) && ((k_uptime_get_32() - startTime) > timeout_ms))
    {
      break;
    }
    k_busy_wait(1000); // Sleep for 1ms
  }

  if (cal_ready == NAU7802_CAL_SUCCESS)
  {
    return (true);
  }
  return (false);
}

//Set the readings per second
//10, 20, 40, 80, and 320 samples per second is available
bool setSampleRate(uint8_t rate)
{
  if (rate > 0b111)
    rate = 0b111; //Error check

  uint8_t value;
  getRegister(NAU7802_CTRL2, &value);
  value &= 0b10001111; //Clear CRS bits
  value |= rate << 4;  //Mask in new CRS bits

  return (setRegister(NAU7802_CTRL2, value));
}

//Select between 1 and 2
bool setChannel(uint8_t channelNumber)
{
  if (channelNumber == NAU7802_CHANNEL_1)
    return (clearBit(NAU7802_CTRL2_CHS, NAU7802_CTRL2)); //Channel 1 (default)
  else
    return (setBit(NAU7802_CTRL2_CHS, NAU7802_CTRL2)); //Channel 2
}

//Power up digital and analog sections of scale
bool powerUp()
{
  setBit(NAU7802_PU_CTRL_PUD, NAU7802_PU_CTRL);
  setBit(NAU7802_PU_CTRL_PUA, NAU7802_PU_CTRL);

  //Wait for Power Up bit to be set - takes approximately 200us
  uint8_t counter = 0;
  while (1)
  {
    if (getBit(NAU7802_PU_CTRL_PUR, NAU7802_PU_CTRL) == true)
      break; //Good to go
    k_busy_wait(1000); // Sleep for 1ms
    if (counter++ > 100)
      return (false); //Error
  }
  return (setBit(NAU7802_PU_CTRL_CS, NAU7802_PU_CTRL)); // Set Cycle Start bit. See 9.1 point 5
}

//Puts scale into low-power mode
bool powerDown()
{
  clearBit(NAU7802_PU_CTRL_PUD, NAU7802_PU_CTRL);
  return (clearBit(NAU7802_PU_CTRL_PUA, NAU7802_PU_CTRL));
}

//Resets all registers to Power Of Defaults
bool reset()
{
  setBit(NAU7802_PU_CTRL_RR, NAU7802_PU_CTRL); //Set RR
  k_busy_wait(1000); // Sleep for 1ms
  return (clearBit(NAU7802_PU_CTRL_RR, NAU7802_PU_CTRL)); //Clear RR to leave reset state
}

//Set the onboard Low-Drop-Out voltage regulator to a given value
//2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.2, 4.5V are available
bool setLDO(uint8_t ldoValue)
{
  if (ldoValue > 0b111)
    ldoValue = 0b111; //Error check

  //Set the value of the LDO
  uint8_t value;
  getRegister(NAU7802_CTRL1, &value);
  value &= 0b11000111;    //Clear LDO bits
  value |= ldoValue << 3; //Mask in new LDO bits
  setRegister(NAU7802_CTRL1, value);

  return (setBit(NAU7802_PU_CTRL_AVDDS, NAU7802_PU_CTRL)); //Enable the internal LDO
}

void setLDORampDelay(unsigned long delay)
{
  _ldoRampDelay = delay;
}
unsigned long getLDORampDelay()
{
  return _ldoRampDelay;
}

//Set the gain
//x1, 2, 4, 8, 16, 32, 64, 128 are avaialable
bool setGain(uint8_t gainValue)
{
  if (gainValue > 0b111)
    gainValue = 0b111; //Error check

  uint8_t value;
  getRegister(NAU7802_CTRL1, &value);
  value &= 0b11111000; //Clear gain bits
  value |= gainValue;  //Mask in new bits

  return (setRegister(NAU7802_CTRL1, value));
}

//Get the revision code of this IC
uint8_t getRevisionCode()
{
  uint8_t revisionCode;
  getRegister(NAU7802_DEVICE_REV, &revisionCode);
  return (revisionCode & 0x0F);
}

//Returns 24-bit reading
//Assumes CR Cycle Ready bit (ADC conversion complete) has been checked to be 1
int32_t getReading()
{
   int32_t value;
   if(!get24BitRegister(NAU7802_ADCO_B2, &value)) {
      return 0;
   }
  return value;
}

//Return the average of a given number of readings
//Gives up after 1000ms so don't call this function to average 8 samples setup at 1Hz output (requires 8s)
int32_t getAverage(uint8_t averageAmount, unsigned long timeout_ms)
{
  int32_t total = 0; // Readings are 24-bit. We're good to average 255 if needed
  uint8_t samplesAquired = 0;

  unsigned long startTime = k_uptime_get_32();
  while (1)
  {
    if (available() == true)
    {
      total += getReading();
      if (++samplesAquired == averageAmount)
        break; //All done
    }
    if (k_uptime_get_32() - startTime > timeout_ms)
      return (0); //Timeout - Bail with error
    k_busy_wait(1000); // Sleep for 1ms
  }
  total /= averageAmount;

  return (total);
}

//Call when scale is setup, level, at running temperature, with nothing on it
void calculateZeroOffset(uint8_t averageAmount, unsigned long timeout_ms)
{
  setZeroOffset(getAverage(averageAmount, timeout_ms));
}

//Sets the internal variable. Useful for users who are loading values from NVM.
void setZeroOffset(int32_t newZeroOffset)
{
  _zeroOffset = newZeroOffset;
}

int32_t getZeroOffset()
{
  return (_zeroOffset);
}

//Call after zeroing. Provide the float weight sitting on scale. Units do not matter.
void calculateCalibrationFactor(float weightOnScale, uint8_t averageAmount, unsigned long timeout_ms)
{
  int32_t onScale = getAverage(averageAmount, timeout_ms);
  float newCalFactor = ((float)(onScale - _zeroOffset)) / weightOnScale;
  setCalibrationFactor(newCalFactor);
}

//Pass a known calibration factor into library. Helpful if users is loading settings from NVM.
//If you don't know your cal factor, call setZeroOffset(), then calculateCalibrationFactor() with a known weight
void setCalibrationFactor(float newCalFactor)
{
  _calibrationFactor = newCalFactor;
}

float getCalibrationFactor()
{
  return (_calibrationFactor);
}

//Returns the y of y = mx + b using the current weight on scale, the cal factor, and the offset.
float getWeight(bool allowNegativeWeights, uint8_t samplesToTake, unsigned long timeout_ms)
{
  int32_t onScale = getAverage(samplesToTake, timeout_ms);

  //Prevent the current reading from being less than zero offset
  //This happens when the scale is zero'd, unloaded, and the load cell reports a value slightly less than zero value
  //causing the weight to be negative or jump to millions of pounds
  if (allowNegativeWeights == false)
  {
    if (onScale < _zeroOffset)
      onScale = _zeroOffset; //Force reading to zero
  }

  float weight = ((float)(onScale - _zeroOffset)) / _calibrationFactor;
  return (weight);
}

//Set Int pin to be high when data is ready (default)
bool setIntPolarityHigh()
{
  return (clearBit(NAU7802_CTRL1_CRP, NAU7802_CTRL1)); //0 = CRDY pin is high active (ready when 1)
}

//Set Int pin to be low when data is ready
bool setIntPolarityLow()
{
  return (setBit(NAU7802_CTRL1_CRP, NAU7802_CTRL1)); //1 = CRDY pin is low active (ready when 0)
}

//Mask & set a given bit within a register
bool setBit(uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value;
  getRegister(registerAddress, &value);
  value |= (1 << bitNumber); //Set this bit
  return (setRegister(registerAddress, value));
}

//Mask & clear a given bit within a register
bool clearBit(uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value; 
  getRegister(registerAddress, &value);
  value &= ~(1 << bitNumber); //Set this bit
  return (setRegister(registerAddress, value));
}

//Return a given bit within a register
bool getBit(uint8_t bitNumber, uint8_t registerAddress)
{
  uint8_t value;
  getRegister(registerAddress, &value);
  value &= (1 << bitNumber); //Clear all but this bit
  return (value);
}

//Get contents of a 24-bit signed register (conversion result and offsets)
bool get24BitRegister(uint8_t registerAddress, int32_t *value)
{
   uint8_t data[3];
   uint8_t reg2 = NAU7802_ADCO_B2;
   if(i2c_write_read(i2c_dev, I2C_ADDR, &registerAddress, 1, data, sizeof(data)) != 0) {
      return false;
   }

   union
   {
      uint32_t unsigned32;
      int32_t signed32;
   } signedUnsigned32; // Avoid ambiguity

   signedUnsigned32.unsigned32 = (uint32_t)data[0] << 16; //MSB
   signedUnsigned32.unsigned32 |= (uint32_t)data[1] << 8; //MidSB
   signedUnsigned32.unsigned32 |= (uint32_t)data[2];      //LSB

   if ((signedUnsigned32.unsigned32 & 0x00800000) == 0x00800000)
   signedUnsigned32.unsigned32 |= 0xFF000000; // Preserve 2's complement

   *value = signedUnsigned32.signed32;

  return true;
}

//Send 24 LSBs of value to given register address. Return true if successful
//Note: assumes bit 23 is already correct for 24-bit signed
bool set24BitRegister(uint8_t registerAddress, int32_t value)
{
  union
  {
    uint32_t unsigned32;
    int32_t signed32;
  } signedUnsigned32; // Avoid ambiguity

  signedUnsigned32.signed32 = value;

  uint8_t data[3] = {signedUnsigned32.unsigned32 >> 16 & 0xFF, 
                     signedUnsigned32.unsigned32 >> 8 & 0xFF,
                     signedUnsigned32.unsigned32 & 0xFF};
  if(i2c_burst_write(i2c_dev, I2C_ADDR, registerAddress, data, sizeof(data)) != 0) {
    return false;
  }

  return (true);
}

//Get contents of a 32-bit register (gains)
bool get32BitRegister(uint8_t registerAddress, uint32_t *value)
{
   uint8_t data[4];
   if(i2c_write_read(i2c_dev, I2C_ADDR, &registerAddress, 1, data, sizeof(data)) != 0) {
      return false;
   }

   *value = (uint32_t)data[0] << 24; //MSB
   *value |= (uint32_t)data[1] << 16;
   *value |= (uint32_t)data[2] << 8;
   *value |= (uint32_t)data[3];      //LSB

  return true;
}

//Send 32-bit value to given register address. Return true if successful
bool set32BitRegister(uint8_t registerAddress, uint32_t value)
{
   uint8_t data[4] = {((value >> 24) & 0xFF), 
                      ((value >> 16) & 0xFF),
                      ((value >> 8) & 0xFF),
                      (value & 0xFF)};
   if(i2c_burst_write(i2c_dev, I2C_ADDR, registerAddress, data, sizeof(data)) != 0) {
      return false;
   }
  return (true);
}

//Helper methods
int32_t getChannel1Offset() { 
   int32_t value;
   if(!get24BitRegister(NAU7802_OCAL1_B2, &value)) {
      return 0;
   }
   return value; 
}
bool setChannel1Offset(int32_t value) { return set24BitRegister(NAU7802_OCAL1_B2, value); }
uint32_t getChannel1Gain() {
   int32_t value;
   if(!get32BitRegister(NAU7802_GCAL1_B3, &value)) {
      return 0;
   }
   return value; 
}
bool setChannel1Gain(uint32_t value) { return set32BitRegister(NAU7802_GCAL1_B3, value); }