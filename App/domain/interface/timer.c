#include "timer.h"

WEAK_LINK AppResult Timer_Initialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK AppResult Timer_Deinitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void Timer_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        info->code = APP_ERROR_NOT_IMPLEMENT;
    }
}

WEAK_LINK void* Timer_Create(void (*callback)(void* context), bool is_singleshot, void* context) {
    (void)callback;
    (void)is_singleshot;
    (void)context;
    return NULL;
}

WEAK_LINK AppResult Timer_Dispose(void* instance) {
    (void)instance;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult Timer_Start(void* instance, uint32_t tick, bool is_singleshot, void* context) {
    (void)instance;
    (void)tick;
    (void)is_singleshot;
    (void)context;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult Timer_Stop(void* instance) {
    (void)instance;
    return APP_RESULT_FAIL;
}

WEAK_LINK bool Timer_IsRunning(void* instance) {
    (void)instance;
    return false;
}