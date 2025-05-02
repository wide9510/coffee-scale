#ifndef LOADCELL_H
#define LOADCELL_H

#include <zephyr/kernel.h>


bool loadcell_init(void);
bool loadcell_available(void);
void get_weighted_average(float new_sample, float previous_average, float* weighted_average);
void get_raw_adc_value(int32_t* adc_raw);
void get_weighted_adc_value(float* weighted_adc);
void get_grams_value(float* grams);

#endif // LOADCELL_H