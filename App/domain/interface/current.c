#include "current.h"

WEAK_LINK AppResult Current_Initialize(const BmsConfig* config) {
    (void)config;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult Current_Deinitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void Current_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        info->code = APP_ERROR_NOT_IMPLEMENT;
    }
}

WEAK_LINK AppResult Current_GetData(BmsPackState pack_state, BmsCurrentData* data) {
    (void)data;
    return APP_RESULT_FAIL;
}
