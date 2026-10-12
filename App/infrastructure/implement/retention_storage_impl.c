#include <string.h>

#include "../../domain/interface/retention_storage.h"
#include "../service/platform.h"
#include "../service/rtc_port.h"

#define self (retention_storage_private)

#define DATA_EMPTY (0xFFFFFFFF)

static struct {
    AppErrorInfo error_info;
} retention_storage_private;

AppResult RetentionStorage_Initialize() {
    if (!RtcPort_IsInitialized()) {
        if (RtcPort_Initialize() != APP_RESULT_OK) {
            RtcPort_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    return APP_RESULT_OK;
}

AppResult RetentionStorage_Deinitialize() {
    if (RtcPort_IsInitialized()) {
        if (RtcPort_Deinitialize() != APP_RESULT_OK) {
            RtcPort_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    return APP_RESULT_OK;
}

void RetentionStorage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

bool RetentionStorage_IsFormatOccurred(void) { return RtcPort_IsBackupFormatOccurred(); }

AppResult RetentionStorage_GetInfo(RetentionStorageInfo* info) {
    if (info == NULL) {
        return APP_RESULT_INVALID_ARG;
    }

    RtcPort_GetBackupRegisterInfo(&info->key_count, &info->data_size);
    return APP_RESULT_OK;
}

AppResult RetentionStorage_Save(uint8_t key, uint8_t* data, uint8_t size) {
    uint32_t buffer = 0;

    memcpy(&buffer, data, size > sizeof(uint32_t) ? sizeof(uint32_t) : size);

    if (RtcPort_WriteBackupRegister(key, buffer) != APP_RESULT_OK) {
        RtcPort_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult RetentionStorage_Load(uint8_t key, uint8_t* data, uint8_t size) {
    uint32_t buffer = 0;

    if (RtcPort_ReadBackupRegister(key, &buffer) != APP_RESULT_OK) {
        RtcPort_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    memcpy(data, &buffer, size > sizeof(uint32_t) ? sizeof(uint32_t) : size);

    return APP_RESULT_OK;
}

AppResult RetentionStorage_Erase(uint8_t key) {
    uint32_t buffer = DATA_EMPTY;

    if (RtcPort_WriteBackupRegister(key, buffer) != APP_RESULT_OK) {
        RtcPort_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}
