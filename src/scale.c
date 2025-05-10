#include <zephyr/kernel.h>
#include "error.h"
#include "nau7802.h"
#include "scale.h"
#include "display_data.h"

#define SMOOTHING_FACTOR 0.1f


K_MSGQ_DEFINE(display_msgq, sizeof(struct display_data), 10, 4);

static float getFlowRate(float weight, float last_weight, int32_t time, int32_t last_time)
{
   float flow_rate = (weight - last_weight) / ((time - last_time)/1000.0f); // g/s
   if(flow_rate < 0) {
      flow_rate = 0;
   }
   return flow_rate;
}

extern struct k_mutex nau_mutex;

void scale_run(void)
{
   if(!begin(true)) {
      printk("Loadcell initialization failed\n");
      return -1;
   }

   int32_t time = 0;
   int32_t last_time = 0;
   float weight = 0.0f;
   float last_weight = 0.0f;
   int32_t last_refresh = 0;

   int32_t timer_start = 0;
   bool timer_started = false;
   int32_t elapsed_time = 0;

   int32_t avg = 0;
   while (1) {
      if(available()) {
         time = k_uptime_get();
         // avg = getAverage(5, 75);
         weight = getWeight(true, 5, 75);

         int time_next = k_uptime_get();

         // float flow_rate = getFlowRate(weight, last_weight, time, last_time);

         struct display_data data = {
            .weight = weight,
         };
         k_msgq_put(&display_msgq, &data, K_NO_WAIT);
         
         if(time - last_refresh > 50) {
            printk("\033[2J"); // Clear screen
            printk("\033[H");  // Move cursor to top-left
            printk("Time taken: %d ms |   ", time_next - time);
            // printk("ADC Value: %d | ", avg);
            printk("%.2f g    |  ", (double)weight);
            // printk("%.2f g/s  |  ", (double)flow_rate);
            // if(timer_started){
            //    printk("%d\n", (time - timer_start)/1000);
            // }
            // else {
            //    printk("%d\n", elapsed_time/1000);
            // }
            last_refresh = time;
         }
         last_time = time;
         last_weight = weight;
      }
      k_sleep(K_MSEC(50));
   }
   return;
}