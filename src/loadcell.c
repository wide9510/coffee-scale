#include "loadcell.h"
#include "nau7802.h"

#define SMOOTHING_FACTOR 0.5f

float tare = 0.0f;
float last_average = 0.0f;
float calibration = 0.0f;

bool loadcell_init(void)
{
   return begin(true);
}

bool loadcell_available(void)
{
   return available();
}

void get_weighted_average(float new_sample, float previous_average, float* weighted_average)
{
   *weighted_average = (SMOOTHING_FACTOR * new_sample) + ((1 - SMOOTHING_FACTOR) * previous_average); 
}
void get_raw_adc_value(int32_t* adc_raw)
{
   *adc_raw = getReading();
}
void get_weighted_adc_value(float* weighted_adc)
{
   int32_t adc_raw;
   get_raw_adc_value(&adc_raw);
   get_weighted_average((float)adc_raw, last_average, weighted_adc);
   last_average = *weighted_adc; // Update last average
}

void get_grams_value(float* grams)
{
   float weighted_adc;
   get_weighted_adc_value(&weighted_adc);
   *grams = (weighted_adc - tare) / calibration;
}