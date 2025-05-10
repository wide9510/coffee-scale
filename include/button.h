#ifndef BUTTON_H
#define BUTTON_H

#include "error.h"
#include <zephyr/drivers/gpio.h>



status_t init_button(const struct gpio_dt_spec *button, struct gpio_callback *callback, gpio_callback_handler_t handler);

status_t init_tare_button();
status_t init_calibration_button();

#endif // BUTTON_H