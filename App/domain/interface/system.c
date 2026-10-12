#include "./system.h"

WEAK_LINK void System_Reset(void) {}

WEAK_LINK uint32_t System_GetUniqueId(uint8_t index) {
    (void)index;
    return 0;
}

WEAK_LINK void System_GetResetReason(AppResetReason* reset_reason, uint32_t* raw_reset_flag) {
    (void)reset_reason;
    (void)raw_reset_flag;
}

WEAK_LINK void System_ClearResetReason(void) {}