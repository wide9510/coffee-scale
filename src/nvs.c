#include "nvs.h"

struct nvs_fs fs;

#define SMOOTHING_FACTOR 0.3f

extern float tare, last_average, calibration;

void load_nvs_data(void)
{
    int rc;

    rc = nvs_read(&fs, STORAGE_ID_TARE, &tare, sizeof(tare));
    if (rc < 0) {
        printk("Failed to read tare: %d\n", rc);
        tare = 0.0f; // Default value
    } else {
        printk("Tare loaded: %f\n", (double)tare);
    }

    rc = nvs_read(&fs, STORAGE_ID_CALIBRATION, &calibration, sizeof(calibration));
    if (rc < 0) {
        printk("Failed to read calibration: %d\n", rc);
        calibration = 1.0f; // Default value
    } else {
        printk("Calibration loaded: %f\n", (double)calibration);
    }
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