#include "retention_storage.h"

WEAK_LINK AppResult RetentionStorage_Initialize() { return APP_RESULT_FAIL; }

WEAK_LINK AppResult RetentionStorage_Deinitialize() { return APP_RESULT_FAIL; }

WEAK_LINK void RetentionStorage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        APP_ERROR_SET_INFO(*info, APP_ERROR_NOT_IMPLEMENT);
    }
}

WEAK_LINK bool RetentionStorage_IsFormatOccurred(void) { return false; }

WEAK_LINK AppResult RetentionStorage_Save(uint8_t key, uint8_t* data, uint8_t size) {
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult RetentionStorage_Load(uint8_t key, uint8_t* data, uint8_t size) {
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult RetentionStorage_Erase(uint8_t key) { return APP_RESULT_FAIL; }

WEAK_LINK bool RetentionStorage_IsEmpty(uint8_t key) { return true; }