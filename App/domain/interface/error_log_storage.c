#include "error_log_storage.h"

WEAK_LINK AppResult ErrorLogStorage_Initialize() { return APP_RESULT_FAIL; }

WEAK_LINK AppResult ErrorLogStorage_Deinitialize() { return APP_RESULT_FAIL; }

WEAK_LINK void ErrorLogStorage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        info->code = APP_ERROR_NOT_IMPLEMENT;
    }
}

WEAK_LINK AppResult ErrorLogStorage_Record(const AppErrorInfo* error_info) {
    return APP_RESULT_FAIL;
}
