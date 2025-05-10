#ifndef ERR_GPIO_H
#define ERR_GPIO_H
#include "error.h"

status_t convert_gpio_err(int code) {
   switch (code) {
       case 0: return STATUS_OK;
       case -EINVAL: return STATUS_GPIO_INVALID;
       case -ENOTSUP: return STATUS_GPIO_UNSUPPORTED;
       case -EIO: return STATUS_GPIO_IO;
       case -EWOULDBLOCK: return STATUS_GPIO_WOULDBLOCK;
       default: return STATUS_GPIO_FAIL;
   }
}


#endif // ERR_GPIO_H