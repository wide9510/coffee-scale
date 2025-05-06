#include "nvs.h"
#include <zephyr/fs/nvs.h>
#include "nau7802.h"

struct nvs_fs fs;

#define SMOOTHING_FACTOR 0.3f

void write_calibration_factor(float calibration_factor)
{
    nvs_write(&fs, STORAGE_ID_CALIBRATION, &calibration_factor, sizeof(calibration_factor));
}
void write_tare(float tare_value)
{
   nvs_write(&fs, STORAGE_ID_TARE, &tare_value, sizeof(tare_value));
}

void load_nvs_data(void)
{
    int rc;
    int32_t tare = 0;
    rc = nvs_read(&fs, STORAGE_ID_TARE, &tare, sizeof(tare));
    if (rc < 0) {
        printk("Failed to read tare: %d\n", rc);
    } else {
        printk("Tare loaded: %f\n", (double)tare);
    }
    setZeroOffset(tare);

    float calibration = 1.0f;
    rc = nvs_read(&fs, STORAGE_ID_CALIBRATION, &calibration, sizeof(calibration));
    if (rc < 0) {
        printk("Failed to read calibration: %d\n", rc);
    } else {
        printk("Calibration loaded: %f\n", (double)calibration);
    }
      setCalibrationFactor(calibration);
}

void init_nvs(void)
{
   int rc;
   
   fs.flash_device = NVS_PARTITION_DEVICE;
   fs.offset = FLASH_AREA_OFFSET(storage);
   fs.sector_size = 4096; // 4kB
   fs.sector_count = 2;

    rc = nvs_mount(&fs);
    if (rc) {
        printk("Failed to mount NVS: %d\n", rc);
        return;
    }

    printk("NVS mounted successfully\n");
}