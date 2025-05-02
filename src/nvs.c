#include "nvs.h"

struct nvs_fs fs;

#define SMOOTHING_FACTOR 0.3f

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