#include "../../domain/interface/system.h"
#include "../service/platform.h"
#include "../service/rtc_port.h"

#define BOOT_RESET_FLAGS_MASK                                                                     \
    (RCC_CSR_LPWRRSTF | RCC_CSR_WWDGRSTF | RCC_CSR_IWDGRSTF | RCC_CSR_SFTRSTF | RCC_CSR_PORRSTF | \
     RCC_CSR_PINRSTF | RCC_CSR_BORRSTF)

void System_Reset(void) { NVIC_SystemReset(); }

uint32_t System_GetUniqueId(uint8_t index) {
    switch (index) {
        case 0:
            return HAL_GetUIDw0();
        case 1:
            return HAL_GetUIDw1();
        case 2:
            return HAL_GetUIDw2();
        default:
            return 0;
    }
}

void System_GetResetReason(AppResetReason* reset_reason, uint32_t* raw_reset_flag) {
    AppResetReason reason = APP_RESET_REASON_UNKNOWN;
    uint32_t flag = RCC->CSR & BOOT_RESET_FLAGS_MASK;

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
        reason = APP_RESET_REASON_INDEPENDENT_WATCHDOG_RESET;

    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) {
        reason = APP_RESET_REASON_WINDOW_WATCHDOG_RESET;

    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
        reason = APP_RESET_REASON_SOFTWARE_RESET;

    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST)) {
        reason = APP_RESET_REASON_POWER_ON_RESET;

    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) {
        reason = APP_RESET_REASON_PIN_RESET;

    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) {
        reason = APP_RESET_REASON_LOW_POWER_RESET;

    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST)) {
        reason = APP_RESET_REASON_BROWN_OUT_RESET;
    }

    *reset_reason = reason;
    *raw_reset_flag = flag;
}

void System_ClearResetReason(void) { __HAL_RCC_CLEAR_RESET_FLAGS(); }