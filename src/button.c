#include "button.h"
#include "error.h"
#include "nvs.h"
#include <zephyr/fs/nvs.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
// LOG_MODULE_REGISTER(error_handling, LOG_LEVEL_ERR);

struct gpio_callback tare_cb_data,
                 calibration_cb_data,
                 clear_tare_cb_data;

static const struct gpio_dt_spec tare_button = GPIO_DT_SPEC_GET(TARE_BUTTON, gpios);
static const struct gpio_dt_spec calibration_button = GPIO_DT_SPEC_GET(CALIBRATION_BUTTON, gpios);
static const struct gpio_dt_spec clear_tare_button = GPIO_DT_SPEC_GET(CLEAR_TARE_BUTTON, gpios);

extern float tare,
             last_average,
             calibration;

extern struct nvs_fs fs;

static void tare_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    tare = last_average;
    int rc = nvs_write(&fs, STORAGE_ID_TARE, &tare, sizeof(tare));
    if (rc < 0) {
        printk("Failed to write tare: %d\n", rc);
    } else {
        printk("Tare set to: %f\n", (double)tare);
    }
}
static void calibration_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    calibration = (last_average - tare)/144.0f;
    int rc = nvs_write(&fs, STORAGE_ID_CALIBRATION, &calibration, sizeof(calibration));
    if (rc < 0) {
        printk("Failed to write calibration: %d\n", rc);
    } else {
        printk("Calibration set to: %f\n", (double)calibration);
    }
}

static void clear_tare_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    tare = 0;
    int rc = nvs_write(&fs, STORAGE_ID_TARE, &tare, sizeof(tare));
    if (rc < 0) {
        printk("Failed to write tare: %d\n", rc);
    } else {
        printk("Tare cleared\n");
    }
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
    init_button(&tare_button, &tare_cb_data, tare_button_pressed);
}
void init_calibration_button()
{
    init_button(&calibration_button, &calibration_cb_data, calibration_button_pressed);
}
void init_clear_tare_button()
{
   init_button(&clear_tare_button, &clear_tare_cb_data, clear_tare_button_pressed);
}