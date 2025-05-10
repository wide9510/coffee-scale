#include "display.h"
#include "display_data.h"
#include <zephyr/display/cfb.h>
#include <zephyr/kernel.h>
#include "error.h"
LOG_MODULE_REGISTER(display, LOG_LEVEL_ERR);

#define CFB_DISPLAY_HEIGHT CFB_DISPLAY_HEIGH

#define DISPLAY DT_NODELABEL(ssd1306)
static const struct device* dev = DEVICE_DT_GET(DISPLAY);

extern struct k_msgq display_msgq;

status_t display_init(void)
{

   //TODO: check if the display is ready
   device_is_ready(dev);

   EXPECT_OK(display_set_pixel_format(dev, PIXEL_FORMAT_MONO10));
   EXPECT_OK(cfb_framebuffer_init(dev));
   EXPECT_OK(cfb_framebuffer_clear(dev, true));
   EXPECT_OK(display_blanking_off(dev));
   EXPECT_OK(cfb_framebuffer_set_font(dev, 0));
   EXPECT_OK(cfb_set_kerning(dev, 3));

   return STATUS_OK;
}

void display_run(void)
{

   char                buffer[7];
   struct display_data recv_data;
   while (1)
   {
      if (k_msgq_get(&display_msgq, &recv_data, K_NO_WAIT) == 0)
      {
         snprintk(buffer, sizeof(buffer), "%.02f g", (double) recv_data.weight);
         cfb_framebuffer_clear(dev, false);
         cfb_draw_text(dev, buffer, 0, 0);
         // if (cfb_print(dev, buffer, 0, 0))
         // {
         //    printf("Failed to print a string\n");
         //    continue;
         // }
         cfb_framebuffer_finalize(dev);
      }
      k_sleep(K_MSEC(50));
   }
   return;
}