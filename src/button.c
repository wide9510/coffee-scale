#include "button.h"
#include "error.h"
#include "nvs.h"
#include <zephyr/fs/nvs.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include "nau7802.h"
// LOG_MODULE_REGISTER(error_handling, LOG_LEVEL_ERR);

struct gpio_callback tare_cb_data,
                 calibration_cb_data,
                 clear_tare_cb_data;

static struct k_work calibration_work;
static struct k_work tare_work;

static const struct gpio_dt_spec tare_button = GPIO_DT_SPEC_GET(TARE_BUTTON, gpios);
static const struct gpio_dt_spec calibration_button = GPIO_DT_SPEC_GET(CALIBRATION_BUTTON, gpios);

bool isCalibrated = false;

extern struct nvs_fs fs;

void tare_work_handler(struct k_work *work)
{
   calculateZeroOffset(1,10);
   int32_t tare = getZeroOffset();
   write_tare(tare);
}

static void tare_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
   k_work_submit(&tare_work);
}

void calibration_work_handler(struct k_work *work)
{
   calculateCalibrationFactor(144, 1, 10);
   float calibration = getCalibrationFactor();
   write_calibration_factor(calibration);
}

static void calibration_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
   k_work_submit(&calibration_work);
}

void init_button(const struct gpio_dt_spec* button, struct gpio_callback* cb_data, gpio_callback_handler_t handler)
{
   if(!device_is_ready(button->port)) {
      printk ("GPIO device not ready\n");
   }
   int rc = gpio_pin_configure_dt(button, GPIO_INPUT);
   if(rc < 0) {
      printk("Failed to configure GPIO pin %d: %d\n", button->pin, rc);
   }
   rc = gpio_pin_interrupt_configure_dt(button, GPIO_INT_EDGE_TO_ACTIVE);
   if(rc < 0) {
      printk("Failed to configure GPIO interrupt for pin %d: %d\n", button->pin, rc);
   }
   gpio_init_callback(cb_data, handler, BIT(button->pin));
   gpio_add_callback(button->port, cb_data);
}

void init_tare_button()
{  
   k_work_init(&tare_work, tare_work_handler);
   init_button(&tare_button, &tare_cb_data, tare_button_pressed);
}
void init_calibration_button()
{
   k_work_init(&calibration_work, calibration_work_handler);
   init_button(&calibration_button, &calibration_cb_data, calibration_button_pressed);
}