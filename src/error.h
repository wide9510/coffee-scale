#ifndef ERROR_H
#define ERROR_H

// #include <zephyr/logging/log.h>

// #define EXPECT_OK(expr, err_code) \
//     do { \
//         if ((expr) <= 0) { \
//             LOG_ERR("Failed at %s:%d (%s)", __FILE__, __LINE__, #expr); \
//             return (err_code); \
//         } \
//     } while (0)

typedef enum {
    STATUS_OK = 0,
    ERR_DEV_NOT_READY,
    ERR_GPIO_CONFIG_FAIL,
    ERR_GPIO_INT_FAIL,
    ERR_GPIO_CB_FAIL,
    ERR_UNKNOWN,
} status_t;

#endif // ERROR_H