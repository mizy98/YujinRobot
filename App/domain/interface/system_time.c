#include "system_time.h"

WEAK_LINK AppResult SystemTime_Initialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK AppResult SystemTime_Deinitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void SystemTime_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        info->code = APP_ERROR_NOT_IMPLEMENT;
    }
}

WEAK_LINK AppResult SystemTime_SetTimestamp(uint64_t timestamp) { return APP_RESULT_FAIL; }

WEAK_LINK AppResult SystemTime_GetTimestamp(uint64_t* timestamp) {
    (void)timestamp;
    return APP_RESULT_FAIL;
}

WEAK_LINK uint32_t SystemTime_GetTick(void) { return 0; }