#include "thermostat.h"

#include "../interface/signal.h"
#include "../interface/system_time.h"

#define self (thermostat_private)

static struct {
    AppErrorInfo error_info;

    uint32_t fan_check_start_tick;
    uint32_t heater_check_start_tick;

    BmsDeviceState fan_state;
    BmsDeviceState heater_state;

    BmsFanConfig fan_config;
    BmsHeaterConfig heater_config;

    ThermostatCallback callback;
} thermostat_private;

static void _processFan(uint32_t current_tick, int16_t temperature) {
    switch (self.fan_state) {
        case BMS_DEVICE_STATE_OFF: {
            if (temperature > self.fan_config.threshold_value) {
                if (self.fan_check_start_tick == 0) {
                    self.fan_check_start_tick = current_tick;

                } else if ((current_tick - self.fan_check_start_tick) >=
                           self.fan_config.threshold_tick) {
                    Signal_SetFan(true);

                    self.fan_state = BMS_DEVICE_STATE_ON;
                    self.fan_check_start_tick = 0;

                    if (self.callback.onFanStateChanged != NULL) {
                        self.callback.onFanStateChanged(self.fan_state);
                    }
                }
            } else {
                self.fan_check_start_tick = 0;
            }
            break;
        }

        case BMS_DEVICE_STATE_ON: {
            if (temperature < self.fan_config.release_threshold_value) {
                if (self.fan_check_start_tick == 0) {
                    self.fan_check_start_tick = current_tick;

                } else if ((current_tick - self.fan_check_start_tick) >=
                           self.fan_config.release_threshold_tick) {
                    Signal_SetFan(false);

                    self.fan_state = BMS_DEVICE_STATE_OFF;
                    self.fan_check_start_tick = 0;

                    if (self.callback.onFanStateChanged != NULL) {
                        self.callback.onFanStateChanged(self.fan_state);
                    }
                }
            } else {
                self.fan_check_start_tick = 0;
            }

            break;
        }

        case BMS_DEVICE_STATE_ERROR: {
            break;
        }
    }
}

static void _processHeater(uint32_t current_tick, int16_t temperature) {
    switch (self.heater_state) {
        case BMS_DEVICE_STATE_OFF: {
            if (temperature < self.heater_config.threshold_value) {
                if (self.heater_check_start_tick == 0) {
                    self.heater_check_start_tick = current_tick;

                } else if ((current_tick - self.heater_check_start_tick) >=
                           self.heater_config.threshold_tick) {
                    Signal_SetHeater(true);

                    self.heater_state = BMS_DEVICE_STATE_ON;
                    self.heater_check_start_tick = 0;

                    if (self.callback.onHeaterStateChanged != NULL) {
                        self.callback.onHeaterStateChanged(self.heater_state);
                    }
                }
            } else {
                self.heater_check_start_tick = 0;
            }
            break;
        }

        case BMS_DEVICE_STATE_ON: {
            if (temperature > self.heater_config.release_threshold_value) {
                if (self.heater_check_start_tick == 0) {
                    self.heater_check_start_tick = current_tick;

                } else if ((current_tick - self.heater_check_start_tick) >=
                           self.heater_config.release_threshold_tick) {
                    Signal_SetHeater(false);

                    self.heater_state = BMS_DEVICE_STATE_OFF;
                    self.heater_check_start_tick = 0;

                    if (self.callback.onHeaterStateChanged != NULL) {
                        self.callback.onHeaterStateChanged(self.heater_state);
                    }
                }
            } else {
                self.heater_check_start_tick = 0;
            }
            break;
        }

        case BMS_DEVICE_STATE_ERROR: {
            break;
        }
    }
}

AppResult Thermostat_Initialize(const BmsConfig* config, const ThermostatCallback* callback) {
    if (config == NULL || callback == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_THERMOSTAT_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    Signal_SetFan(false);
    self.fan_state = BMS_DEVICE_STATE_OFF;
    self.fan_check_start_tick = 0;

    Signal_SetHeater(false);
    self.heater_state = BMS_DEVICE_STATE_OFF;
    self.heater_check_start_tick = 0;

    self.fan_config = config->fan;
    self.heater_config = config->heater;

    self.callback = *callback;

    return APP_RESULT_OK;
}

AppResult Thermostat_Deinitialize(void) { return APP_RESULT_OK; }

AppResult Thermostat_SetConfig(const BmsConfig* config) {
    if (config == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_THERMOSTAT_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    Signal_SetFan(false);
    self.fan_state = BMS_DEVICE_STATE_OFF;
    self.fan_check_start_tick = 0;

    Signal_SetHeater(false);
    self.heater_state = BMS_DEVICE_STATE_OFF;
    self.heater_check_start_tick = 0;

    self.fan_config = config->fan;
    self.heater_config = config->heater;

    return APP_RESULT_OK;
}

void Thermostat_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

void Thermostat_Process(BmsPackTemperatureData* data) {
    uint32_t current_tick = SystemTime_GetTick();

    if (data == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_THERMOSTAT_INVALID_ARG);
        return;
    }

    if (self.fan_config.is_enabled) {
        _processFan(current_tick, data->max_temperature);
    }

    if (self.heater_config.is_enabled) {
        _processHeater(current_tick, data->min_temperature);
    }
}