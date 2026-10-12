#include "nvs.h"

#include "../interface/retention_storage.h"

#define self (nvs_private)

#define SOC_KEY (0u)
#define CYCLE_KEY (1u)
#define SOC_TIMESTAMP_KEY (2u)
#define PF_STATE_KEY (3u)
#define SOH_KEY (4u)

static struct {
    AppErrorInfo error_info;

    RetentionStorageInfo storage_info;
} nvs_private;

AppResult Nvs_Initialize(void) {
    if (RetentionStorage_Initialize() != APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (RetentionStorage_GetInfo(&self.storage_info) != APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult Nvs_Deinitialize(void) {
    if (RetentionStorage_Deinitialize() != APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void Nvs_GetLastErrorInfo(AppErrorInfo* error_info) {
    if (error_info != NULL) {
        *error_info = self.error_info;
    }
}

bool Nvs_IsFormatOccurred(void) { return RetentionStorage_IsFormatOccurred(); }

AppResult Nvs_SaveSoc(uint32_t timestamp, uint16_t soc) {
    uint8_t buffer[4] = {0};
    buffer[0] = (uint8_t)(soc & 0xFF);
    buffer[1] = (uint8_t)((soc >> 8) & 0xFF);

    if (RetentionStorage_Save(SOC_KEY, buffer, sizeof(buffer)) != APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    buffer[0] = (uint8_t)(timestamp & 0xFF);
    buffer[1] = (uint8_t)((timestamp >> 8) & 0xFF);
    buffer[2] = (uint8_t)((timestamp >> 16) & 0xFF);
    buffer[3] = (uint8_t)((timestamp >> 24) & 0xFF);

    if (RetentionStorage_Save(SOC_TIMESTAMP_KEY, buffer, sizeof(buffer)) != APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult Nvs_LoadSoc(uint32_t* timestamp, uint16_t* soc) {
    uint8_t buffer[4];
    if (RetentionStorage_Load(SOC_KEY, buffer, sizeof(buffer)) != APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    *soc = (uint16_t)(buffer[0] | (buffer[1] << 8));

    if (RetentionStorage_Load(SOC_TIMESTAMP_KEY, buffer, sizeof(buffer)) != APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    *timestamp = (uint32_t)(buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24));

    return APP_RESULT_OK;
}

AppResult Nvs_SaveCycle(float cycle) {
    if (RetentionStorage_Save(CYCLE_KEY, (uint8_t*)&cycle, sizeof(cycle)) != APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult Nvs_LoadCycle(float* cycle) {
    if (RetentionStorage_Load(CYCLE_KEY, (uint8_t*)cycle, sizeof(*cycle)) != APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult Nvs_SaveSoh(float soh) {
    if (RetentionStorage_Save(SOH_KEY, (uint8_t*)&soh, sizeof(soh)) != APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult Nvs_LoadSoh(float* soh) {
    if (RetentionStorage_Load(SOH_KEY, (uint8_t*)soh, sizeof(*soh)) != APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult Nvs_SavePfState(uint32_t pf_state) {
    if (RetentionStorage_Save(PF_STATE_KEY, (uint8_t*)&pf_state, sizeof(pf_state)) !=
        APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult Nvs_LoadPfState(uint32_t* pf_state) {
    if (RetentionStorage_Load(PF_STATE_KEY, (uint8_t*)pf_state, sizeof(*pf_state)) !=
        APP_RESULT_OK) {
        RetentionStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}
