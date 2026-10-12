
#include <time.h>

#include "../../domain/interface/system_time.h"
#include "../service/rtc_port.h"

#define self (system_time_private)

static struct {
    AppErrorInfo error_info;
} system_time_private;

AppResult SystemTime_Initialize(void) {
    if (!RtcPort_IsInitialized()) {
        if (RtcPort_Initialize() != APP_RESULT_OK) {
            RtcPort_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    return APP_RESULT_OK;
}

AppResult SystemTime_Deinitialize(void) {
    if (RtcPort_IsInitialized()) {
        if (RtcPort_Deinitialize() != APP_RESULT_OK) {
            RtcPort_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    return APP_RESULT_OK;
}

void SystemTime_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult SystemTime_SetTimestamp(uint64_t timestamp) {
    if (RtcPort_SetTimestamp(timestamp) != APP_RESULT_OK) {
        RtcPort_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult SystemTime_GetTimestamp(uint64_t* timestamp) {
    if (RtcPort_GetTimestamp(timestamp) != APP_RESULT_OK) {
        RtcPort_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

uint32_t SystemTime_GetTick(void) {
    uint32_t tick = 0;

#ifdef _WIN32
    tick = (uint32_t)GetTickCount64();

#elif defined(USE_HAL_DRIVER)
    tick = HAL_GetTick();

#endif

    return tick;
}
