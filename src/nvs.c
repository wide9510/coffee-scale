#include "nvs.h"
#include "err_nvs.h"
#include "nau7802.h"
#include <zephyr/fs/nvs.h>
#include <zephyr/storage/flash_map.h>

LOG_MODULE_REGISTER(nvs, LOG_LEVEL_ERR);

#define STORAGE_ID_TARE 0x01
#define STORAGE_ID_CALIBRATION 0x02

#define NVS_PARTITION         storage_partition
#define NVS_PARTITION_DEVICE  FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET  FIXED_PARTITION_OFFSET(NVS_PARTITION)

struct nvs_fs fs;

status_t init_nvs(void)
{
   fs.flash_device = NVS_PARTITION_DEVICE;
   fs.offset       = FLASH_AREA_OFFSET(storage);
   fs.sector_size  = 4096; // 4kB
   fs.sector_count = 2;

   EXPECT_OK(nvs_mount(&fs));
   return STATUS_OK;
}

status_t write_calibration_factor(float calibration_factor)
{
   EXPECT_OK(nvs_write(&fs, STORAGE_ID_CALIBRATION, &calibration_factor,
             sizeof(calibration_factor)), convert_nvs_err);
   return STATUS_OK;
}

status_t write_tare(int32_t tare_value)
{
   EXPECT_OK(nvs_write(&fs, STORAGE_ID_TARE, &tare_value, sizeof(tare_value)), convert_nvs_err);
   return STATUS_OK;
}

status_t load_nvs_data(void)
{
   int32_t tare        = 0;
   float   calibration = 1.0f;

   EXPECT_OK(nvs_read(&fs, STORAGE_ID_TARE, &tare, sizeof(tare)),
             convert_nvs_err);
   EXPECT_OK(
       nvs_read(&fs, STORAGE_ID_CALIBRATION, &calibration, sizeof(calibration)),
       convert_nvs_err);

   setZeroOffset(tare);
   setCalibrationFactor(calibration);
   return STATUS_OK;
}
