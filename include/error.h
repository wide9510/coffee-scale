#ifndef ERROR_H
#define ERROR_H

#include <zephyr/logging/log.h>

typedef enum {
   STATUS_OK = 0,
   STATUS_GPIO_INVALID,
   STATUS_GPIO_UNSUPPORTED,
   STATUS_GPIO_IO,
   STATUS_GPIO_FAIL,
   STATUS_GPIO_WOULDBLOCK,
} status_t;

static inline status_t default_convert_error_fn(int code)
{
   return code;
}

#define EXPECT_OK_CONVERT_2(expr, convert_error_fn) \
    do { \
        int _ret = (expr); \
        status_t _err = convert_error_fn(_ret); \
        if (_err != STATUS_OK) { \
            LOG_ERR("Failed at %s:%d (%s)", __FILE__, __LINE__, #expr); \
            return _err; \
        } \
    } while (0)

#define EXPECT_OK_CONVERT_1(expr) EXPECT_OK_CONVERT_2(expr, default_convert_error_fn)

#define GET_EXPECT_OK_CONVERT_MACRO(_1, _2, NAME, ...) NAME

#define EXPECT_OK(...) \
    GET_EXPECT_OK_CONVERT_MACRO(__VA_ARGS__, EXPECT_OK_CONVERT_2, EXPECT_OK_CONVERT_1)(__VA_ARGS__)

#endif // ERROR_H