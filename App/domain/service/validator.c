#include "validator.h"

AppResult Validator_ValidateBmsConfig(const BmsConfig* config, AppErrorInfo* error_info) {
    if (config == NULL) {
        if (error_info != NULL) {
            APP_ERROR_SET_INFO(*error_info, APP_ERROR_VALIDATOR_INVALID_CONFIG);
        }
        return APP_RESULT_FAIL;
    }

    // meta
    if (config->meta.version != APP_CONFIG_VERSION) {
        if (error_info != NULL) {
            APP_ERROR_SET_INFO(*error_info, APP_ERROR_VALIDATOR_APP_VERSION);
        }
        return APP_RESULT_FAIL;
    }

    if (config->meta.size != BMS_CONFIG_OCCUPIED_SIZE) {
        if (error_info != NULL) {
            APP_ERROR_SET_INFO(*error_info, APP_ERROR_VALIDATOR_SIZE);
        }
        return APP_RESULT_FAIL;
    }

    // current
    if (config->state.charge_threshold_current <= config->state.discharge_threshold_current) {
        if (error_info != NULL) {
            APP_ERROR_SET_INFO(*error_info, APP_ERROR_VALIDATOR_INVALID_CONFIG);
        }
        return APP_RESULT_FAIL;
    }

    if (config->state.charge_parse_mode == BMS_CHARGE_PARSE_MODE_TIME &&
        config->state.charge_threshold_tick == 0) {
        if (error_info != NULL) {
            APP_ERROR_SET_INFO(*error_info, APP_ERROR_VALIDATOR_INVALID_CONFIG);
        }
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}