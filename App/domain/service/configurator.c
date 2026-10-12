#include "configurator.h"

#include "../interface/config_storage.h"
#include "validator.h"

#define self (configurator_private)

static struct {
    AppErrorInfo error_info;

    BmsConfig config;
} configurator_private;

AppResult Configurator_Initialize() {
    if (ConfigStorage_Initialize() != APP_RESULT_OK) {
        ConfigStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    BmsConfig config = {0};

    if (ConfigStorage_Load(&config) != APP_RESULT_OK) {
        ConfigStorage_GetLastErrorInfo(&self.error_info);
        if (self.error_info.code != APP_ERROR_CONFIG_STORAGE_EMPTY) {
            return APP_RESULT_FAIL;
        }
    }

    if (Validator_ValidateBmsConfig(&config, &self.error_info) != APP_RESULT_OK) {
        config = BmsConfig_CreateDefault();

        if (ConfigStorage_Save(&config) != APP_RESULT_OK) {
            ConfigStorage_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    self.config = config;

    return APP_RESULT_OK;
}

AppResult Configurator_Deinitialize() {
    if (ConfigStorage_Deinitialize() != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }
    return APP_RESULT_OK;
}

void Configurator_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult Configurator_Set(const BmsConfig* config) {
    if (Validator_ValidateBmsConfig(config, &self.error_info) != APP_RESULT_OK) {
        return APP_RESULT_INVALID_ARG;
    }

    self.config = *config;

    return APP_RESULT_OK;
}

void Configurator_Get(BmsConfig* config) {
    if (config != NULL) {
        *config = self.config;
    }
}

AppResult Configurator_Save(const BmsConfig* config) {
    if (Validator_ValidateBmsConfig(config, &self.error_info) != APP_RESULT_OK) {
        return APP_RESULT_INVALID_ARG;
    }

    if (ConfigStorage_Save(config) != APP_RESULT_OK) {
        ConfigStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    self.config = *config;

    return APP_RESULT_OK;
}

AppResult Configurator_Load(BmsConfig* config) {
    if (config == NULL) {
        return APP_RESULT_INVALID_ARG;
    }

    if (ConfigStorage_Load(config) != APP_RESULT_OK) {
        ConfigStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (Validator_ValidateBmsConfig(config, &self.error_info) != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    self.config = *config;

    return APP_RESULT_OK;
}

AppResult Configurator_EraseAll(void) {
    if (ConfigStorage_EraseAll() != APP_RESULT_OK) {
        ConfigStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    self.config = BmsConfig_CreateDefault();

    return APP_RESULT_OK;
}
