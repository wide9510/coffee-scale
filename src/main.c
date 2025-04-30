#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include "nau7802.h"

int main(void)
{
   if (!begin(true)) {
      printk("NAU7802 initialization failed\n");
      return -1;
   }

    while (1) {
      if(available()) {
        int32_t adc = 0;
        adc = getReading();
        printk("ADC: %d\n\n", adc);
      }
      k_sleep(K_MSEC(100));
    }
   return 0;
}