#include "protection.h"

#include "../interface/system_time.h"
#include "threshold_control.h"

#define self protection_private

#define ODVP_CHECK_EXIT_THRESHOLD_HYSTERESIS (50)  // mV

typedef struct {
    void* handle;
    bool is_over_trigger;
    BmsProtectionConfig config;
    bool is_active;
} ProtectionConfigEntry;

static struct {
    AppErrorInfo error_info;

    union {
        struct {
            struct {
                ProtectionConfigEntry ovp;
                ProtectionConfigEntry uvp;
            } voltage;

            struct {
                ProtectionConfigEntry cocp;
                ProtectionConfigEntry docp;
                ProtectionConfigEntry rocp;
            } current;

            struct {
                ProtectionConfigEntry otp;
                ProtectionConfigEntry cotp;
                ProtectionConfigEntry dotp;
                ProtectionConfigEntry rotp;
                ProtectionConfigEntry utp;
                ProtectionConfigEntry cutp;
                ProtectionConfigEntry dutp;
                ProtectionConfigEntry rutp;
            } temperature;

            struct {
                ProtectionConfigEntry odvp;
                ProtectionConfigEntry codvp;
                ProtectionConfigEntry dodvp;
                ProtectionConfigEntry rodvp;
            } dv;

            struct {
                ProtectionConfigEntry odtp;
            } dt;

            struct {
                ProtectionConfigEntry opp;
                ProtectionConfigEntry upp;
            } pressure;
        };
        ProtectionConfigEntry entries[BMS_PROTECTION_CODE_MAX];
    } config;

    void* last_checked_current_handle;
    void* last_checked_over_temperature_handle;
    void* last_checked_under_temperature_handle;
    void* last_checked_dv_handle;

    uint8_t is_odvp_checking;

    bool is_odvp_check_threshold_used;
    uint32_t odvp_check_enter_threshold;
    uint32_t odvp_check_exit_threshold;

    BmsPackState last_pack_state;

    BmsProtectionState pf_state;

    ProtectionCallback callback;
} protection_private = {
    .error_info = {0},
    .config =
        {
            .voltage = {{0}},
            .current = {{0}},
            .temperature = {{0}},
            .dv = {{0}},
            .dt = {{0}},
            .pressure = {{0}},
        },
    .callback = {0},
};

static void _onProtectionStateChanged(void* context, bool is_active, int32_t snapshot_value) {
    ProtectionConfigEntry* entry = (ProtectionConfigEntry*)context;

    entry->is_active = is_active;

    if (is_active) {
        if (entry->config.is_pf_enabled) {
            if (self.callback.onPermanentFailure != NULL) {
                self.callback.onPermanentFailure((BmsProtectionCode)entry->config.code,
                                                 snapshot_value, entry->config.threshold_value);
            }
        } else {
            if (self.callback.onProtectionOccurred != NULL) {
                self.callback.onProtectionOccurred((BmsProtectionCode)entry->config.code,
                                                   snapshot_value, entry->config.threshold_value);
            }
        }
    } else {
        if (entry->config.is_pf_enabled) {
            return;
        } else {
            if (self.callback.onProtectionReleased != NULL) {
                self.callback.onProtectionReleased((BmsProtectionCode)entry->config.code,
                                                   snapshot_value,
                                                   entry->config.release_threshold_value);
            }
        }
    }
}

static void _checkVoltage(BmsData* data) {
    ThresholdControl_CheckValue(self.config.voltage.ovp.handle,
                                (int32_t)data->voltage.max_bank_voltage);
    ThresholdControl_CheckValue(self.config.voltage.uvp.handle,
                                (int32_t)data->voltage.min_bank_voltage);
}

static void _checkCurrent(BmsData* data, bool is_state_changed) {
    if (is_state_changed) {
        ThresholdControl_ResetDuration(self.last_checked_current_handle);

        switch (data->pack_state) {
            case BMS_PACK_STATE_CHARGE:
            case BMS_PACK_STATE_CHARGE_CC:
            case BMS_PACK_STATE_CHARGE_CV:
                self.last_checked_current_handle = self.config.current.cocp.handle;
                break;

            case BMS_PACK_STATE_DISCHARGE:
                self.last_checked_current_handle = self.config.current.docp.handle;
                break;

            case BMS_PACK_STATE_REGENERATE:
                self.last_checked_current_handle = self.config.current.rocp.handle;
                break;

            default:
                self.last_checked_current_handle = NULL;
                return;
        }
    }

    ThresholdControl_CheckValue(self.last_checked_current_handle, (int32_t)data->current.value);
}

static void _checkTemperature(BmsData* data, bool is_state_changed) {
    if (is_state_changed) {
        ThresholdControl_ResetDuration(self.last_checked_over_temperature_handle);
        ThresholdControl_ResetDuration(self.last_checked_under_temperature_handle);

        switch (data->pack_state) {
            case BMS_PACK_STATE_IDLE:
            case BMS_PACK_STATE_SLEEP:
                self.last_checked_over_temperature_handle = self.config.temperature.otp.handle;
                self.last_checked_under_temperature_handle = self.config.temperature.utp.handle;
                break;

            case BMS_PACK_STATE_CHARGE:
            case BMS_PACK_STATE_CHARGE_CC:
            case BMS_PACK_STATE_CHARGE_CV:
                self.last_checked_over_temperature_handle = self.config.temperature.cotp.handle;
                self.last_checked_under_temperature_handle = self.config.temperature.cutp.handle;
                break;

            case BMS_PACK_STATE_DISCHARGE:
                self.last_checked_over_temperature_handle = self.config.temperature.dotp.handle;
                self.last_checked_under_temperature_handle = self.config.temperature.dutp.handle;
                break;

            case BMS_PACK_STATE_REGENERATE:
                self.last_checked_over_temperature_handle = self.config.temperature.rotp.handle;
                self.last_checked_under_temperature_handle = self.config.temperature.rutp.handle;
                break;

            default:
                self.last_checked_over_temperature_handle = NULL;
                self.last_checked_under_temperature_handle = NULL;
                return;
        }
    }

    ThresholdControl_CheckValue(self.last_checked_over_temperature_handle,
                                (int32_t)data->pack_temperature.max_temperature);
    ThresholdControl_CheckValue(self.last_checked_under_temperature_handle,
                                (int32_t)data->pack_temperature.min_temperature);
}

static void _checkDv(BmsData* data, bool is_state_changed) {
    bool is_need_check = true;

    if (self.is_odvp_check_threshold_used) {
        if (self.is_odvp_checking) {
            is_need_check = !(data->voltage.min_bank_voltage < self.odvp_check_exit_threshold);
        } else {
            is_need_check = (data->voltage.min_bank_voltage > self.odvp_check_enter_threshold);
        }
    }

    self.is_odvp_checking = is_need_check;

    if (is_state_changed) {
        ThresholdControl_ResetDuration(self.last_checked_dv_handle);

        switch (data->pack_state) {
            case BMS_PACK_STATE_IDLE:
            case BMS_PACK_STATE_SLEEP:
                self.last_checked_dv_handle = self.config.dv.odvp.handle;
                break;

            case BMS_PACK_STATE_CHARGE:
            case BMS_PACK_STATE_CHARGE_CC:
            case BMS_PACK_STATE_CHARGE_CV:
                self.last_checked_dv_handle = self.config.dv.codvp.handle;
                break;

            case BMS_PACK_STATE_DISCHARGE:
                self.last_checked_dv_handle = self.config.dv.dodvp.handle;
                break;

            case BMS_PACK_STATE_REGENERATE:
                self.last_checked_dv_handle = self.config.dv.rodvp.handle;
                break;

            default:
                self.last_checked_dv_handle = NULL;
                return;
        }
    }

    if (is_need_check) {
        ThresholdControl_CheckValue(self.last_checked_dv_handle, (int32_t)data->voltage.dv);
    } else {
        ThresholdControl_ResetDuration(self.last_checked_dv_handle);
    }
}

static void _checkDt(BmsData* data) {
    ThresholdControl_CheckValue(self.config.dt.odtp.handle, (int32_t)data->pack_temperature.dt);
}

static void _checkPressure(BmsData* data) {
    ThresholdControl_CheckValue(self.config.pressure.opp.handle, (int32_t)data->pressure);
    ThresholdControl_CheckValue(self.config.pressure.upp.handle, (int32_t)data->pressure);
}

static AppResult _updateConfig(BmsConfig* config) {
    self.config.voltage.ovp.config = config->protection.ovp;
    self.config.voltage.ovp.is_over_trigger = true;
    self.config.voltage.uvp.config = config->protection.uvp;
    self.config.voltage.uvp.is_over_trigger = false;

    self.config.current.cocp.config = config->protection.cocp;
    self.config.current.cocp.is_over_trigger = true;
    self.config.current.docp.config = config->protection.docp;
    self.config.current.docp.is_over_trigger = false;
    self.config.current.rocp.config = config->protection.rocp;
    self.config.current.rocp.is_over_trigger = true;

    self.config.temperature.otp.config = config->protection.otp;
    self.config.temperature.otp.is_over_trigger = true;
    self.config.temperature.cotp.config = config->protection.cotp;
    self.config.temperature.cotp.is_over_trigger = true;
    self.config.temperature.dotp.config = config->protection.dotp;
    self.config.temperature.dotp.is_over_trigger = true;
    self.config.temperature.rotp.config = config->protection.rotp;
    self.config.temperature.rotp.is_over_trigger = true;

    self.config.temperature.utp.config = config->protection.utp;
    self.config.temperature.utp.is_over_trigger = false;
    self.config.temperature.cutp.config = config->protection.cutp;
    self.config.temperature.cutp.is_over_trigger = false;
    self.config.temperature.dutp.config = config->protection.dutp;
    self.config.temperature.dutp.is_over_trigger = false;
    self.config.temperature.rutp.config = config->protection.rutp;
    self.config.temperature.rutp.is_over_trigger = false;

    self.config.dv.odvp.config = config->protection.odvp;
    self.config.dv.odvp.is_over_trigger = true;
    self.config.dv.codvp.config = config->protection.codvp;
    self.config.dv.codvp.is_over_trigger = true;
    self.config.dv.dodvp.config = config->protection.dodvp;
    self.config.dv.dodvp.is_over_trigger = true;
    self.config.dv.rodvp.config = config->protection.rodvp;
    self.config.dv.rodvp.is_over_trigger = true;

    self.config.dt.odtp.config = config->protection.odtp;
    self.config.dt.odtp.is_over_trigger = true;

    self.config.pressure.opp.config = config->protection.opp;
    self.config.pressure.opp.is_over_trigger = true;
    self.config.pressure.upp.config = config->protection.upp;
    self.config.pressure.upp.is_over_trigger = false;

    for (uint8_t i = 0; i < ARRAY_COUNT(self.config.entries); ++i) {
        if (self.config.entries[i].handle != NULL) {
            ThresholdControl_Destroy(self.config.entries[i].handle);
            self.config.entries[i].handle = NULL;
        }

        void* handle = ThresholdControl_Create(
            &(ThresholdControlConfig){
                .is_enabled = self.config.entries[i].config.is_enabled,
                .off_enabled = self.config.entries[i].config.is_release_enabled,
                .is_over_trigger = self.config.entries[i].is_over_trigger,
                .on_threshold = self.config.entries[i].config.threshold_value,
                .off_threshold = self.config.entries[i].config.release_threshold_value,
                .on_duration_tick = self.config.entries[i].config.threshold_tick,
                .off_duration_tick = self.config.entries[i].config.release_threshold_tick,
            },
            &(ThresholdControlCallback){
                .onStateChanged = _onProtectionStateChanged,
            },
            &self.config.entries[i], self.config.entries[i].is_active);
        if (handle == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_PROTECTION_THRESHOLD_UPDATE);
            return APP_RESULT_FAIL;
        }

        self.config.entries[i].handle = handle;
    }

    self.is_odvp_check_threshold_used = config->misc.protection_extension.odvp_threshold > 0;
    self.odvp_check_enter_threshold = config->misc.protection_extension.odvp_threshold;
    self.odvp_check_exit_threshold =
        (config->misc.protection_extension.odvp_threshold > ODVP_CHECK_EXIT_THRESHOLD_HYSTERESIS)
            ? (config->misc.protection_extension.odvp_threshold -
               ODVP_CHECK_EXIT_THRESHOLD_HYSTERESIS)
            : 0;

    return APP_RESULT_OK;
}

AppResult Protection_Initialize(BmsConfig* config, BmsProtectionState pf_state,
                                ProtectionCallback* callback) {
    if (ThresholdControl_Initialize(
            &(ThresholdControlInterface){.getSystemTick = SystemTime_GetTick}) != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_PROTECTION_THRESHOLD_INIT);
        return APP_RESULT_FAIL;
    }

    for (uint8_t i = 0; i < ARRAY_COUNT(self.config.entries); ++i) {
        self.config.entries[i].is_active = false;
    }

    if (_updateConfig(config) != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    self.callback = *callback;

    self.pf_state = pf_state;
    if (self.pf_state.value > 0) {
        for (uint8_t i = 0; i < BMS_PROTECTION_CODE_MAX; ++i) {
            if ((self.pf_state.value & (1 << i)) != 0) {
                ProtectionConfigEntry* entry = &self.config.entries[i];
                if (entry->config.is_pf_enabled) {
                    if (self.callback.onPermanentFailure != NULL) {
                        self.callback.onPermanentFailure((BmsProtectionCode)entry->config.code, 0,
                                                         entry->config.threshold_value);
                    }
                }
            }
        }
    }

    self.last_pack_state = (BmsPackState)0xFF;
    self.last_checked_current_handle = NULL;
    self.last_checked_over_temperature_handle = NULL;
    self.last_checked_under_temperature_handle = NULL;
    self.last_checked_dv_handle = NULL;
    self.is_odvp_checking = false;

    return APP_RESULT_OK;
}

void Protection_GetLastErrorInfo(AppErrorInfo* error_info) {
    if (error_info != NULL) {
        *error_info = self.error_info;
    }
}

AppResult Protection_UpdateConfig(BmsConfig* config) {
    if (_updateConfig(config) != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void Protection_Check(BmsData* data) {
    bool is_state_changed = data->pack_state != self.last_pack_state;

    if (is_state_changed) {
        self.last_pack_state = data->pack_state;
    }

    _checkVoltage(data);
    _checkCurrent(data, is_state_changed);
    _checkTemperature(data, is_state_changed);
    _checkDv(data, is_state_changed);
    _checkDt(data);
    _checkPressure(data);
}

void Protection_SetPfState(BmsProtectionState pf_state) { self.pf_state = pf_state; }

void Protection_GetPfState(BmsProtectionState* pf_state) {
    if (pf_state != NULL) {
        *pf_state = self.pf_state;
    }
}

void Protection_ResetPfState(void) { self.pf_state.value = 0; }

bool Protection_IsTriggered(BmsProtectionCode code, BmsData* data) {
    switch (code) {
        case BMS_PROTECTION_CODE_OVP:
            return ThresholdControl_IsTriggered(self.config.voltage.ovp.handle,
                                                (int32_t)data->voltage.max_bank_voltage);

        case BMS_PROTECTION_CODE_UVP:
            return ThresholdControl_IsTriggered(self.config.voltage.uvp.handle,
                                                (int32_t)data->voltage.min_bank_voltage);

        case BMS_PROTECTION_CODE_COCP:
            return ThresholdControl_IsTriggered(self.config.current.cocp.handle,
                                                (int32_t)data->current.value);

        case BMS_PROTECTION_CODE_DOCP:
            return ThresholdControl_IsTriggered(self.config.current.docp.handle,
                                                (int32_t)data->current.value);

        case BMS_PROTECTION_CODE_ROCP:
            return ThresholdControl_IsTriggered(self.config.current.rocp.handle,
                                                (int32_t)data->current.value);

        case BMS_PROTECTION_CODE_OTP:
            return ThresholdControl_IsTriggered(self.config.temperature.otp.handle,
                                                (int32_t)data->pack_temperature.max_temperature);

        case BMS_PROTECTION_CODE_COTP:
            return ThresholdControl_IsTriggered(self.config.temperature.cotp.handle,
                                                (int32_t)data->pack_temperature.max_temperature);

        case BMS_PROTECTION_CODE_DOTP:
            return ThresholdControl_IsTriggered(self.config.temperature.dotp.handle,
                                                (int32_t)data->pack_temperature.max_temperature);

        case BMS_PROTECTION_CODE_ROTP:
            return ThresholdControl_IsTriggered(self.config.temperature.rotp.handle,
                                                (int32_t)data->pack_temperature.max_temperature);

        case BMS_PROTECTION_CODE_UTP:
            return ThresholdControl_IsTriggered(self.config.temperature.utp.handle,
                                                (int32_t)data->pack_temperature.min_temperature);

        case BMS_PROTECTION_CODE_CUTP:
            return ThresholdControl_IsTriggered(self.config.temperature.cutp.handle,
                                                (int32_t)data->pack_temperature.min_temperature);

        case BMS_PROTECTION_CODE_DUTP:
            return ThresholdControl_IsTriggered(self.config.temperature.dutp.handle,
                                                (int32_t)data->pack_temperature.min_temperature);

        case BMS_PROTECTION_CODE_RUTP:
            return ThresholdControl_IsTriggered(self.config.temperature.rutp.handle,
                                                (int32_t)data->pack_temperature.min_temperature);

        case BMS_PROTECTION_CODE_ODVP:
            return ThresholdControl_IsTriggered(self.config.dv.odvp.handle,
                                                (int32_t)data->voltage.dv);

        case BMS_PROTECTION_CODE_CODVP:
            return ThresholdControl_IsTriggered(self.config.dv.codvp.handle,
                                                (int32_t)data->voltage.dv);

        case BMS_PROTECTION_CODE_DODVP:
            return ThresholdControl_IsTriggered(self.config.dv.dodvp.handle,
                                                (int32_t)data->voltage.dv);

        case BMS_PROTECTION_CODE_RODVP:
            return ThresholdControl_IsTriggered(self.config.dv.rodvp.handle,
                                                (int32_t)data->voltage.dv);

        case BMS_PROTECTION_CODE_ODTP:
            return ThresholdControl_IsTriggered(self.config.dt.odtp.handle,
                                                (int32_t)data->pack_temperature.dt);

        case BMS_PROTECTION_CODE_OPP:
            return ThresholdControl_IsTriggered(self.config.pressure.opp.handle,
                                                (int32_t)data->pressure);

        case BMS_PROTECTION_CODE_UPP:
            return ThresholdControl_IsTriggered(self.config.pressure.upp.handle,
                                                (int32_t)data->pressure);

        default:
            return false;
            break;
    }

    return false;
}

void Protection_Reset() {
    for (uint8_t i = 0; i < ARRAY_COUNT(self.config.entries); ++i) {
        if (self.config.entries[i].handle != NULL) {
            ThresholdControl_Reset(self.config.entries[i].handle);
        }
    }
}

void Protection_GetError(AppErrorInfo* error) {
    if (error != NULL) {
        *error = self.error_info;
    }
}
