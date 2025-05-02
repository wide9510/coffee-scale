#include <zephyr/kernel.h>
#include "error.h"
#include "button.h"
#include "nvs.h"
#include "loadcell.h"


int main(void)
{
   if(!loadcell_init()) {
      printk("Loadcell initialization failed\n");
      return -1;
   }

   init_tare_button();
   init_calibration_button();
   init_clear_tare_button();

   init_nvs();
   load_nvs_data();

   while (1) {
      if(loadcell_available()) {
         float weight;
         get_grams_value(&weight);

         printk("\033[2J"); // Clear screen
         printk("\033[H");  // Move cursor to top-left
         printk("%.2f g\n", (double)weight);
      }
      k_sleep(K_MSEC(100));
   }
   return 0;
}