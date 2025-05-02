#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include "nau7802.h"
#include "error.h"
#include "button.h"
#include "nvs.h"

#define SMOOTHING_FACTOR 0.5f

float tare = 0.0f;
float last_average = 0.0f;
float calibration = 0.0f;

extern struct nvs_fs fs;

void get_weighted_average(float new_sample, float previous_average, float* weighted_average)
{
   *weighted_average = (SMOOTHING_FACTOR * new_sample) + ((1 - SMOOTHING_FACTOR) * previous_average); 
}

int main(void)
{
   if (!begin(true)) {
      printk("NAU7802 initialization failed\n");
      return -1;
   }

   init_tare_button();
   init_calibration_button();
   init_clear_tare_button();

   init_nvs();

   int rc = nvs_read(&fs, STORAGE_ID_TARE, &tare, sizeof(tare));
   if (rc < 0){
      printk("No saved tare found, using current reading\n");
      tare = getReading();
      rc = nvs_write(&fs, STORAGE_ID_TARE, &tare, sizeof(tare));
      printk("Writing tare: %f\n", (double)tare);
   }
   else {
      printk("Saved tare found: %f\n", (double)tare);
   }

   rc = nvs_read(&fs, STORAGE_ID_CALIBRATION, &calibration, sizeof(calibration));
   if (rc < 0){
      printk("No saved calibration found, using default value\n");
      calibration = 1.0f;
   }

   float weighted_average;
   while (1) {
      if(available()) {
         int32_t adc_raw = getReading();
         get_weighted_average(adc_raw, last_average, &weighted_average);
         float tared_adc = weighted_average - tare;
         float weight = tared_adc / calibration;

         printk("ADC: %f", (double)weighted_average);
         printk(" |   Tared ADC: %f", (double)tared_adc);
         printk(" |   Weight: %f\n\n", (double)weight);

         last_average = weighted_average;
      }
      k_sleep(K_MSEC(100));
   }
   return 0;
}