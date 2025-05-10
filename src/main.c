#include "button.h"
#include "display.h"
#include "nvs.h"
#include "scale.h"
#include <zephyr/device.h>
#include <zephyr/kernel.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_ERR);

#define WEIGHT_THREAD_PRIORITY 2
#define DISPLAY_THREAD_PRIORITY 5
#define WORKQ_THREAD_PRIORITY 1
#define WORKQ_STACK_SIZE 2048
#define STACK_SIZE 4096

struct k_work_q workq;
K_THREAD_STACK_DEFINE(workq_stack, WORKQ_STACK_SIZE);
struct k_mutex nau_mutex;

void workq_init(void)
{
   k_work_queue_start(&workq, workq_stack, K_THREAD_STACK_SIZEOF(workq_stack),
                      WORKQ_THREAD_PRIORITY, NULL);
}

status_t weight_thread(void)
{
   k_mutex_init(&nau_mutex);
   workq_init();
   // Set up buttons with their ISRs
   EXPECT_OK(init_tare_button());
   EXPECT_OK(init_calibration_button());

   // Prepare the file system
   EXPECT_OK(init_nvs());
   // load_nvs_data();

   // Initialize and start collecting data from the load cell
   scale_run();
   return STATUS_OK;
}

status_t display_thread(void)
{
   EXPECT_OK(display_init());

   display_run();
   return STATUS_OK;
}

K_THREAD_DEFINE(weight_thread_id, STACK_SIZE, weight_thread, NULL, NULL, NULL,
                WEIGHT_THREAD_PRIORITY, 0, 0);

K_THREAD_DEFINE(display_thread_id, STACK_SIZE, display_thread, NULL, NULL, NULL,
                DISPLAY_THREAD_PRIORITY, 0, 0);
