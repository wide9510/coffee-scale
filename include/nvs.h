#ifndef NVS_H
#define NVS_H

#include "error.h"

status_t init_nvs(void);
status_t load_nvs_data(void);
status_t write_calibration_factor(float calibration_factor);
status_t write_tare(int32_t tare_value);

#endif   // NVS_H