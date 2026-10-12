#include "config_storage.h"

WEAK_LINK AppResult ConfigStorage_Initialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK AppResult ConfigStorage_Deinitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void ConfigStorage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        APP_ERROR_SET_INFO(*info, APP_ERROR_NOT_IMPLEMENT);
    }
}

WEAK_LINK AppResult ConfigStorage_Load(BmsConfig* config) {
    (void)config;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult ConfigStorage_Save(const BmsConfig* config) {
    (void)config;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult ConfigStorage_EraseAll(void) { return APP_RESULT_FAIL; }
