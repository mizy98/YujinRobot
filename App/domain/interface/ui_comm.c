#include "ui_comm.h"

WEAK_LINK AppResult UiComm_Initialize(const UiCommCallback* callback) {
    (void)callback;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult UiComm_Deinitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void UiComm_GetLastErrorInfo(AppErrorInfo* error_info) {
    if (error_info != NULL) {
        error_info->code = APP_ERROR_NOT_IMPLEMENT;
    }
}

WEAK_LINK void UiComm_ProcessUsb(void) {}

WEAK_LINK void UiComm_ProcessUart(void) {}

WEAK_LINK AppResult UiComm_SendData(const uint8_t* data, uint16_t size) { return APP_RESULT_FAIL; }
