#include <zephyr/kernel.h>
#include <zephyr/display/cfb.h>
#include "display.h"
#include "display_data.h"

#define CFB_DISPLAY_HEIGHT CFB_DISPLAY_HEIGH

#define DISPLAY DT_NODELABEL(ssd1306)
static const struct device *dev = DEVICE_DT_GET(DISPLAY);

extern struct k_msgq display_msgq;

void display_run(void)
{
   uint16_t x_res;
   uint16_t y_res;
   uint16_t rows;
   uint8_t ppt;

   if (!device_is_ready(dev)) {
      printf("Device %s not ready\n", dev->name);
      return;
   }

   if (display_set_pixel_format(dev, PIXEL_FORMAT_MONO10) != 0) {
      if (display_set_pixel_format(dev, PIXEL_FORMAT_MONO01) != 0) {
         printf("Failed to set required pixel format");
         return;
      }
   }

   printf("Initialized %s\n", dev->name);

   if (cfb_framebuffer_init(dev)) {
      printf("Framebuffer initialization failed!\n");
      return;
   }

   cfb_framebuffer_clear(dev, true);

   display_blanking_off(dev);

   x_res = cfb_get_display_parameter(dev, CFB_DISPLAY_WIDTH);
   y_res = cfb_get_display_parameter(dev, CFB_DISPLAY_HEIGHT);
   rows = cfb_get_display_parameter(dev, CFB_DISPLAY_ROWS);
   ppt = cfb_get_display_parameter(dev, CFB_DISPLAY_PPT);

cfb_framebuffer_set_font(dev, 0);

   printf("x_res %d, y_res %d, ppt %d, rows %d, cols %d\n",
         x_res,
         y_res,
         ppt,
         rows,
         cfb_get_display_parameter(dev, CFB_DISPLAY_COLS));

//  cfb_framebuffer_invert(dev);

   cfb_set_kerning(dev, 3);
   char buffer[7];
   struct display_data recv_data;
   while (1) {
      if(k_msgq_get(&display_msgq, &recv_data, K_NO_WAIT) == 0)
      {
         snprintk(buffer, sizeof(buffer),
                  "%.02f g", (double)recv_data.weight);
         cfb_framebuffer_clear(dev, false);
         if(cfb_print(dev,
                  buffer,
                  0, 0)) {
            printf("Failed to print a string\n");
            continue;
         }
      }
      cfb_framebuffer_finalize(dev);
      k_sleep(K_MSEC(50));
   }
   return;
}