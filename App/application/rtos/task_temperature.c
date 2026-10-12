#include "task_temperature.h"

#include "../../domain/interface/temperature.h"
#include "../../domain/service/thermostat.h"

#define self (_task_temperature)
#define super (_task_temperature.super)

APP_CCMRAM static TaskTemperature _task_temperature;
APP_CCMRAM static uint8_t _stack[TASK_TEMPERATURE_STACK_SIZE];
APP_CCMRAM static App* _app;

static void _onFanStateChanged(BmsDeviceState state) { _app->data.cooler_state = state; }

static void _runner(Rtos* rtos) {
    switch (super.state) {
        case TASK_STATE_RESET: {
            uint32_t flag = EVENT_SYSTEM_READY_STORAGE;
            flag = Rtos_WaitEventNoClear(rtos->event.system_ready, flag, false, UINT32_MAX);
            if (((flag & EVENT_SYSTEM_READY_STORAGE) == 0) || ((flag & RTOS_FLAG_ERROR) != 0)) {
                APP_ERROR_SET_INFO(super.error_info,
                                   APP_ERROR_TASK_TEMPERATURE_SYSTEM_READY_TIMEOUT);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (rtos->event.temperature == NULL) {
                void* event = Rtos_CreateEvent();
                if (event == NULL) {
                    APP_ERROR_SET_INFO(super.error_info, APP_ERROR_STARTUP_RTOS_CREATE_EVENT);
                    super.state = TASK_STATE_ERROR;
                    break;
                } else {
                    rtos->event.temperature = event;
                }
            }

            if (Temperature_Initialize(&rtos->app->config) != APP_RESULT_OK) {
                Temperature_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (Thermostat_Initialize(&rtos->app->config,
                                      &(ThermostatCallback){
                                          .onFanStateChanged = _onFanStateChanged,
                                          .onHeaterStateChanged = NULL,
                                      }) != APP_RESULT_OK) {
                Thermostat_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            BmsPackTemperatureData pack_temperature_data = {0};
            BmsBoardTemperatureData board_temperature_data = {0};

            if (Temperature_GetDatas(&pack_temperature_data, &board_temperature_data, NULL, NULL) !=
                APP_RESULT_OK) {
                Temperature_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            rtos->app->data.pack_temperature = pack_temperature_data;
            rtos->app->data.board_temperature = board_temperature_data;

            Rtos_SetEvent(rtos->event.system_ready, EVENT_SYSTEM_READY_TEMPERATURE);

            super.state = TASK_STATE_RUN;
            break;
        }

        case TASK_STATE_RUN: {
            BmsPackTemperatureData pack_temperature_data = {0};
            BmsBoardTemperatureData board_temperature_data = {0};

            if (Temperature_GetDatas(&pack_temperature_data, &board_temperature_data, NULL, NULL) !=
                APP_RESULT_OK) {
                Temperature_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            rtos->app->data.pack_temperature = pack_temperature_data;
            rtos->app->data.board_temperature = board_temperature_data;

            Thermostat_Process(&pack_temperature_data);

            uint32_t flag = Rtos_WaitEvent(rtos->event.temperature, EVENT_TEMPERATURE_UPDATE_CONFIG,
                                           true, TEMPERATURE_READ_TICK);
            if (flag & RTOS_FLAG_ERROR) {
                break;
            } else if (flag & EVENT_TEMPERATURE_UPDATE_CONFIG) {
                if (Thermostat_SetConfig(&rtos->app->config) != APP_RESULT_OK) {
                    Thermostat_GetLastErrorInfo(&super.error_info);
                    super.state = TASK_STATE_ERROR;
                    break;
                }
            }

            Rtos_SleepMs(TEMPERATURE_READ_TICK);

            break;
        }

        case TASK_STATE_ERROR: {
            (void)Temperature_DeInitialize();
            (void)Thermostat_Deinitialize();
            Rtos_SleepMs(TEMPERATURE_READ_TICK);

            super.state = TASK_STATE_RESET;

            rtos->app->data.last_error_code = super.error_info.code;
            break;
        }
    }
}

static void _wrapper(void* argument) {
    while (true) {
        _runner((Rtos*)argument);
    }
}

AppResult TaskTemperature_Initialize(Rtos* rtos) {
    void* task = Rtos_CreateStaticTask(_wrapper, rtos, "temperature", _stack, sizeof(_stack),
                                       TASK_TEMPERATURE_PRIORITY);
    if (task == NULL) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_TEMPERATURE_CREATE);
        return APP_RESULT_FAIL;
    }

    super.handle = task;
    super.state = TASK_STATE_RESET;
    super.error_info.code = APP_ERROR_NONE;
    super.stack_usage = 0.0f;
    super.stack = _stack;
    super.stack_size = sizeof(_stack);

    rtos->task.temperature = &self;
    _app = rtos->app;

    return APP_RESULT_OK;
}

AppResult TaskTemperature_Deinitialize(void) {
    if (Rtos_DeleteTask(super.handle)) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_TEMPERATURE_DELETE);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void TaskTemperature_GetLastErrorInfo(AppErrorInfo* error_info) {
    if (error_info != NULL) {
        *error_info = super.error_info;
    }
}
