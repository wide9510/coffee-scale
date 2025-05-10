#ifndef ERR_NVS_H
#define ERR_NVS_H

#include "error.h"

status_t convert_nvs_err(int code)
{
   if(code >= 0) return STATUS_OK;
   return code; // nvs uses errno codes
}

#endif // ERR_NVS_H