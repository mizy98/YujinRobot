#include "external_comm.h"

WEAK_LINK AppResult ExternalComm_Initialize(App* app) {
    (void)app;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult ExternalComm_Deinitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void ExternalComm_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        APP_ERROR_SET_INFO(*info, APP_ERROR_NOT_IMPLEMENT);
    }
}

WEAK_LINK AppResult ExternalComm_Process(void) { return APP_RESULT_FAIL; }
