#ifndef _NAU7802_H
#define _NAU7802_H

#include <zephyr/kernel.h>

//Register Map
typedef enum
{
  NAU7802_PU_CTRL = 0x00,
  NAU7802_CTRL1,
  NAU7802_CTRL2,
  NAU7802_OCAL1_B2,
  NAU7802_OCAL1_B1,
  NAU7802_OCAL1_B0,
  NAU7802_GCAL1_B3,
  NAU7802_GCAL1_B2,
  NAU7802_GCAL1_B1,
  NAU7802_GCAL1_B0,
  NAU7802_OCAL2_B2,
  NAU7802_OCAL2_B1,
  NAU7802_OCAL2_B0,
  NAU7802_GCAL2_B3,
  NAU7802_GCAL2_B2,
  NAU7802_GCAL2_B1,
  NAU7802_GCAL2_B0,
  NAU7802_I2C_CONTROL,
  NAU7802_ADCO_B2,
  NAU7802_ADCO_B1,
  NAU7802_ADCO_B0,
  NAU7802_ADC = 0x15, //Shared ADC and OTP 32:24
  NAU7802_OTP_B1,     //OTP 23:16 or 7:0?
  NAU7802_OTP_B0,     //OTP 15:8
  NAU7802_PGA = 0x1B,
  NAU7802_PGA_PWR = 0x1C,
  NAU7802_DEVICE_REV = 0x1F,
} Scale_Registers;

//Bits within the PU_CTRL register
typedef enum
{
  NAU7802_PU_CTRL_RR = 0,
  NAU7802_PU_CTRL_PUD,
  NAU7802_PU_CTRL_PUA,
  NAU7802_PU_CTRL_PUR,
  NAU7802_PU_CTRL_CS,
  NAU7802_PU_CTRL_CR,
  NAU7802_PU_CTRL_OSCS,
  NAU7802_PU_CTRL_AVDDS,
} PU_CTRL_Bits;

//Bits within the CTRL1 register
typedef enum
{
  NAU7802_CTRL1_GAIN = 2,
  NAU7802_CTRL1_VLDO = 5,
  NAU7802_CTRL1_DRDY_SEL = 6,
  NAU7802_CTRL1_CRP = 7,
} CTRL1_Bits;

//Bits within the CTRL2 register
typedef enum
{
  NAU7802_CTRL2_CALMOD = 0,
  NAU7802_CTRL2_CALS = 2,
  NAU7802_CTRL2_CAL_ERROR = 3,
  NAU7802_CTRL2_CRS = 4,
  NAU7802_CTRL2_CHS = 7,
} CTRL2_Bits;

//Bits within the PGA register
typedef enum
{
  NAU7802_PGA_CHP_DIS = 0,
  NAU7802_PGA_INV = 3,
  NAU7802_PGA_BYPASS_EN,
  NAU7802_PGA_OUT_EN,
  NAU7802_PGA_LDOMODE,
  NAU7802_PGA_RD_OTP_SEL,
} PGA_Bits;

//Bits within the PGA PWR register
typedef enum
{
  NAU7802_PGA_PWR_PGA_CURR = 0,
  NAU7802_PGA_PWR_ADC_CURR = 2,
  NAU7802_PGA_PWR_MSTR_BIAS_CURR = 4,
  NAU7802_PGA_PWR_PGA_CAP_EN = 7,
} PGA_PWR_Bits;

//Allowed Low drop out regulator voltages
typedef enum
{
  NAU7802_LDO_2V4 = 0b111,
  NAU7802_LDO_2V7 = 0b110,
  NAU7802_LDO_3V0 = 0b101,
  NAU7802_LDO_3V3 = 0b100,
  NAU7802_LDO_3V6 = 0b011,
  NAU7802_LDO_3V9 = 0b010,
  NAU7802_LDO_4V2 = 0b001,
  NAU7802_LDO_4V5 = 0b000,
} NAU7802_LDO_Values;

//Allowed gains
typedef enum
{
  NAU7802_GAIN_128 = 0b111,
  NAU7802_GAIN_64 = 0b110,
  NAU7802_GAIN_32 = 0b101,
  NAU7802_GAIN_16 = 0b100,
  NAU7802_GAIN_8 = 0b011,
  NAU7802_GAIN_4 = 0b010,
  NAU7802_GAIN_2 = 0b001,
  NAU7802_GAIN_1 = 0b000,
} NAU7802_Gain_Values;

//Allowed samples per second
typedef enum
{
  NAU7802_SPS_320 = 0b111,
  NAU7802_SPS_80 = 0b011,
  NAU7802_SPS_40 = 0b010,
  NAU7802_SPS_20 = 0b001,
  NAU7802_SPS_10 = 0b000,
} NAU7802_SPS_Values;

//Select between channel values
typedef enum
{
  NAU7802_CHANNEL_1 = 0,
  NAU7802_CHANNEL_2 = 1,
} NAU7802_Channels;

//Calibration state
typedef enum
{
  NAU7802_CAL_SUCCESS = 0,
  NAU7802_CAL_IN_PROGRESS = 1,
  NAU7802_CAL_FAILURE = 2,
} NAU7802_Cal_Status;

//Calibration mode
typedef enum
{
  NAU7802_CALMOD_INTERNAL = 0,
  NAU7802_CALMOD_OFFSET = 2,
  NAU7802_CALMOD_GAIN,
} NAU7802_Cal_Mode;

  bool begin(bool reset); //Check communication and initialize sensor
  bool isConnected();                                      //Returns true if device acks at the I2C address

  bool available();                          //Returns true if Cycle Ready bit is set (conversion is complete)
  int32_t getReading();                      //Returns 24-bit reading. Assumes CR Cycle Ready bit (ADC conversion complete) has been checked by .available()
  int32_t getAverage(uint8_t samplesToTake, unsigned long timeout_ms); //Return the average of a given number of readings

  void calculateZeroOffset(uint8_t averageAmount, unsigned long timeout_ms); //Also called taring. Call this with nothing on the scale
  void setZeroOffset(int32_t newZeroOffset);           //Sets the internal variable. Useful for users who are loading values from NVM.
  int32_t getZeroOffset();                             //Ask library for this value. Useful for storing value into NVM.

  void calculateCalibrationFactor(float weightOnScale, uint8_t averageAmount, unsigned long timeout_ms); //Call this with the value of the thing on the scale. Sets the calibration factor based on the weight on scale and zero offset.
  void setCalibrationFactor(float calFactor);                                      //Pass a known calibration factor into library. Helpful if users is loading settings from NVM.
  float getCalibrationFactor();                                                    //Ask library for this value. Useful for storing value into NVM.

  float getWeight(bool allowNegativeWeights, uint8_t samplesToTake, unsigned long timeout_ms); //Once you've set zero offset and cal factor, you can ask the library to do the calculations for you.

  bool setGain(uint8_t gainValue);        //Set the gain. x1, 2, 4, 8, 16, 32, 64, 128 are available
  bool setLDO(uint8_t ldoValue);          //Set the onboard Low-Drop-Out voltage regulator to a given value. 2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.2, 4.5V are avaialable
  void setLDORampDelay(unsigned long delay); //LDO (AVDD) takes ~200ms to ramp up. During .begin, delay for _ldoRampDelay before performing calibrateAFE
  unsigned long getLDORampDelay();
  bool setSampleRate(uint8_t rate);       //Set the readings per second. 10, 20, 40, 80, and 320 samples per second is available
  bool setChannel(uint8_t channelNumber); //Select between 1 and 2

  bool calibrateAFE(NAU7802_Cal_Mode mode);      //Synchronous calibration of the analog front end of the NAU7802. Returns true if CAL_ERR bit is 0 (no error)
  void beginCalibrateAFE(NAU7802_Cal_Mode mode); //Begin asynchronous calibration of the analog front end of the NAU7802. Poll for completion with calAFEStatus() or wait with waitForCalibrateAFE().
  bool waitForCalibrateAFE(unsigned long timeout_ms); //Wait for asynchronous AFE calibration to complete with optional timeout.
  NAU7802_Cal_Status calAFEStatus();                 //Check calibration status.

  bool reset(); //Resets all registers to Power Of Defaults

  bool powerUp();   //Power up digital and analog sections of scale, ~2mA
  bool powerDown(); //Puts scale into low-power 200nA mode

  bool setIntPolarityHigh(); //Set Int pin to be high when data is ready (default)
  bool setIntPolarityLow();  //Set Int pin to be low when data is ready

  uint8_t getRevisionCode(); //Get the revision code of this IC. Always 0x0F.

  bool setBit(uint8_t bitNumber, uint8_t registerAddress);   //Mask & set a given bit within a register
  bool clearBit(uint8_t bitNumber, uint8_t registerAddress); //Mask & clear a given bit within a register
  bool getBit(uint8_t bitNumber, uint8_t registerAddress);   //Return a given bit within a register

  bool get24BitRegister(uint8_t registerAddress, int32_t *value);        //Get contents of a 24-bit signed register (conversion result and offsets)
  bool set24BitRegister(uint8_t registerAddress, int32_t value); //Send 24 LSBs of value to given register address. Return true if successful
  bool get32BitRegister(uint8_t registerAddress, uint32_t *value);       //Get contents of a 32-bit register (gains)
  bool set32BitRegister(uint8_t registerAddress, uint32_t value); //Send a given value to be written to given address. Return true if successful

  int32_t getChannel1Offset();        //Helper method for 
  bool setChannel1Offset(int32_t value); //Send 24 LSBs of value to given register address. Return true if successful
  uint32_t getChannel1Gain();       //Get contents of a 32-bit register (gains)
  bool setChannel1Gain(uint32_t value); //Send a given value to be written to given address. Return true if successful


#endif