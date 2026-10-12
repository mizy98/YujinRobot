#include "../../domain/interface/error_log_storage.h"

#define self (error_log_storage_private)

static struct {
    AppErrorInfo error_info;
} error_log_storage_private;

AppResult ErrorLogStorage_Initialize() { return APP_RESULT_OK; }

AppResult ErrorLogStorage_Deinitialize() { return APP_RESULT_OK; }

void ErrorLogStorage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult ErrorLogStorage_Record(const AppErrorInfo* error_info) { return APP_RESULT_OK; }
