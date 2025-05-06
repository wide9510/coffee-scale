#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "scale.h"
#include "display.h"
#include "button.h"
#include "nvs.h"

#define WEIGHT_THREAD_PRIORITY 1
#define DISPLAY_THREAD_PRIORITY 5
#define STACK_SIZE 4096
 

void weight_thread(void)
{
   // Set up buttons with their ISRs
   init_tare_button();
   init_calibration_button();

   // Prepare the file system 
   init_nvs();
   // load_nvs_data();

   // Initialize and start collecting data from the load cell
   scale_run();
}

void display_thread(void)
{
   display_run();
}

K_THREAD_DEFINE(weight_thread_id, STACK_SIZE,
      weight_thread, NULL, NULL, NULL,
      WEIGHT_THREAD_PRIORITY, 0, 0);

K_THREAD_DEFINE(display_thread_id, STACK_SIZE,
      display_thread, NULL, NULL, NULL,
      DISPLAY_THREAD_PRIORITY, 0, 0);
