#include "button.h"

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/fs/nvs.h>

#include "err_gpio.h"
#include "error.h"
#include "nau7802.h"
#include "nvs.h"

#define TARE_BUTTON DT_ALIAS(sw0)
#define CALIBRATION_BUTTON DT_ALIAS(sw1)
#define CLEAR_TARE_BUTTON DT_ALIAS(sw2)

extern struct k_work_q workq;
LOG_MODULE_REGISTER(button, LOG_LEVEL_ERR);

struct gpio_callback tare_cb_data, calibration_cb_data, clear_tare_cb_data;

static const struct gpio_dt_spec tare_button =
    GPIO_DT_SPEC_GET(TARE_BUTTON, gpios);

static const struct gpio_dt_spec calibration_button =
    GPIO_DT_SPEC_GET(CALIBRATION_BUTTON, gpios);

extern struct nvs_fs fs;

static void tare_work_handler(struct k_work* work)
{
   calculateZeroOffset(10, 1000);
   write_tare(getZeroOffset());
}
K_WORK_DEFINE(tare_work, tare_work_handler);

static void tare_button_pressed(const struct device*  dev,
                                struct gpio_callback* cb, uint32_t pins)
{
   k_work_submit_to_queue(&workq, &tare_work);
}

static void calibration_work_handler(struct k_work* work)
{
   calculateCalibrationFactor(144, 10, 1000);
   write_calibration_factor(getCalibrationFactor());
}

K_WORK_DEFINE(calibration_work, calibration_work_handler);

static void calibration_button_pressed(const struct device*  dev,
                                       struct gpio_callback* cb, uint32_t pins)
{
   k_work_submit_to_queue(&workq, &calibration_work);
}

status_t init_button(const struct gpio_dt_spec* button,
                     struct gpio_callback*      cb_data,
                     gpio_callback_handler_t    handler)
{
   device_is_ready(button->port);
   EXPECT_OK(gpio_pin_configure_dt(button, GPIO_INPUT), convert_gpio_err);
   EXPECT_OK(gpio_pin_interrupt_configure_dt(button, GPIO_INT_EDGE_TO_ACTIVE),
             convert_gpio_err);
   gpio_init_callback(cb_data, handler, BIT(button->pin));
   EXPECT_OK(gpio_add_callback(button->port, cb_data), convert_gpio_err);
   return STATUS_OK;
}

status_t init_tare_button()
{
   EXPECT_OK(init_button(&tare_button, &tare_cb_data, tare_button_pressed));
   return STATUS_OK;
}

status_t init_calibration_button()
{
   EXPECT_OK(init_button(&calibration_button, &calibration_cb_data,
                         calibration_button_pressed));
   return STATUS_OK;
}