#ifndef APP_TYPE_H
#define APP_TYPE_H

#ifdef _MSC_VER
#define WEAK_LINK
#elif defined(__GNUC__)
#define WEAK_LINK __attribute__((weak))
#else
#define WEAK_LINK
#endif

#ifndef ARRAY_COUNT
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

typedef enum {
    APP_RESULT_OK = 0,
    APP_RESULT_FAIL,
    APP_RESULT_TIMEOUT,
    APP_RESULT_INVALID_ARG,
} AppResult;

#endif