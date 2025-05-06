#ifndef NVS_H
#define NVS_H

#include <zephyr/storage/flash_map.h>


#define STORAGE_ID_TARE 0x01
#define STORAGE_ID_CALIBRATION 0x02

#define NVS_PARTITION         storage_partition
#define NVS_PARTITION_DEVICE  FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET  FIXED_PARTITION_OFFSET(NVS_PARTITION)

void init_nvs(void);
void load_nvs_data(void);
void write_calibration_factor(float calibration_factor);
void write_tare(float tare_value);

#endif   // NVS_H