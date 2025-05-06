#ifndef BUTTON_H
#define BUTTON_H

#include "error.h"
#include <zephyr/drivers/gpio.h>

#define TARE_BUTTON DT_ALIAS(sw0)
#define CALIBRATION_BUTTON DT_ALIAS(sw1)
#define CLEAR_TARE_BUTTON DT_ALIAS(sw2)


void init_button(const struct gpio_dt_spec *button, struct gpio_callback *callback, gpio_callback_handler_t handler);

void init_tare_button();
void init_calibration_button();
void init_clear_tare_button();

#endif // BUTTON_H