#ifndef APP_BOOT_INFO_H
#define APP_BOOT_INFO_H

#include <stdbool.h>
#include <stdint.h>

enum _AppResetReason {
    APP_RESET_REASON_UNKNOWN,
    APP_RESET_REASON_PIN_RESET,
    APP_RESET_REASON_POWER_ON_RESET,
    APP_RESET_REASON_SOFTWARE_RESET,
    APP_RESET_REASON_INDEPENDENT_WATCHDOG_RESET,
    APP_RESET_REASON_WINDOW_WATCHDOG_RESET,
    APP_RESET_REASON_LOW_POWER_RESET,
    APP_RESET_REASON_BROWN_OUT_RESET,
};
typedef uint8_t AppResetReason;

typedef struct {
    AppResetReason reset_reason;
    uint32_t reset_flag;
    bool is_backup_valid;

} AppBootInfo;

#endif