#include "task_current.h"

#include "../../domain/interface/current.h"
#include "../../domain/interface/system_time.h"
#include "../../domain/service/nvs.h"
#include "../../domain/service/pack_state.h"
#include "../../domain/service/soc.h"

#define self (_task_current)
#define super (_task_current.super)

APP_CCMRAM static TaskCurrent _task_current;
APP_CCMRAM static uint8_t _stack[TASK_CURRENT_STACK_SIZE];
APP_CCMRAM static Rtos* _rtos;

static void onBmsStateChanged(BmsPackState state) {
    if (state == BMS_PACK_STATE_SLEEP || state == BMS_PACK_STATE_PROTECTION ||
        state == BMS_PACK_STATE_PERMANENT_FAILURE) {
        Rtos_SetEvent(_rtos->event.contactor,
                      EVENT_CONTACTOR_CHARGE_OFF | EVENT_CONTACTOR_DISCHARGE_OFF);
    } else {
        Rtos_SetEvent(_rtos->event.contactor,
                      EVENT_CONTACTOR_CHARGE_ON | EVENT_CONTACTOR_DISCHARGE_ON);
    }
}

static void _runner(Rtos* rtos) {
    switch (super.state) {
        case TASK_STATE_RESET: {
            uint32_t expected_flag = EVENT_SYSTEM_READY_STORAGE | EVENT_SYSTEM_READY_VOLTAGE |
                                     EVENT_SYSTEM_READY_TEMPERATURE;

            uint32_t flag =
                Rtos_WaitEventNoClear(rtos->event.system_ready, expected_flag, false, UINT32_MAX);
            if (((flag & expected_flag) == 0) || ((flag & RTOS_FLAG_ERROR) != 0)) {
                APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_CURRENT_SYSTEM_READY_TIMEOUT);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (Current_Initialize(&rtos->app->config) != APP_RESULT_OK) {
                Current_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (PackState_Initialize(&rtos->app->config, onBmsStateChanged) != APP_RESULT_OK) {
                PackState_GetErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            uint16_t initial_soc = 0;
            uint32_t timestamp = 0;

            float initial_cycle = 0.0f;
            float initial_soh = 0.0f;

            if (Nvs_LoadSoc(&timestamp, &initial_soc) != APP_RESULT_OK) {
                Nvs_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (Nvs_LoadCycle(&initial_cycle) != APP_RESULT_OK) {
                Nvs_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (Nvs_LoadSoh(&initial_soh) != APP_RESULT_OK) {
                Nvs_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            bool is_invalid_soc = initial_soc > 1000;
            if (is_invalid_soc) {
                initial_soc = 1000;
            }

            bool is_invalid_cycle = initial_cycle < 0.0f || initial_cycle > 100000.0f;
            if (is_invalid_cycle) {
                initial_cycle = 0.0f;
            }

            bool is_invalid_soh = initial_soh <= 0.0f || initial_soh > 100.0f;
            if (is_invalid_soh) {
                initial_soh = 100.0f;
            }

            const uint32_t timestamp_day = 24 * 60 * 60;
            uint64_t current_timestamp = 0;
            bool is_old_soc = false;

            if (SystemTime_GetTimestamp(&current_timestamp) == APP_RESULT_OK) {
                is_old_soc = current_timestamp > timestamp &&
                             (current_timestamp - timestamp) > timestamp_day;
            } else {
                is_old_soc = true;
            }

            BmsData data = rtos->app->data;
            uint16_t ocv = data.voltage.average_bank_voltage;

            int16_t temperature = data.pack_temperature.average_temperature;
            bool is_need_force_calibration = is_old_soc && !is_invalid_soc;

            if (Soc_Initialize(&rtos->app->config, initial_soc, initial_cycle, initial_soh, ocv,
                               temperature, is_need_force_calibration) != APP_RESULT_OK) {
                Soc_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            Rtos_SetEvent(rtos->event.system_ready, EVENT_SYSTEM_READY_CURRENT);

            super.state = TASK_STATE_RUN;
            break;
        }

        case TASK_STATE_RUN: {
            BmsData data = rtos->app->data;

            BmsCurrentData current_data = {0};
            if (Current_GetData(data.pack_state, &current_data) != APP_RESULT_OK) {
                Current_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            BmsPackState pack_state = PackState_ParseState(
                current_data.filtered_value, data.voltage.pack_voltage, data.protection_state,
                data.pf_state, data.soc.real_soc, data.voltage.min_bank_voltage);

            BmsSocData soc_data = {0};
            if (Soc_Calculate(&data, &soc_data) != APP_RESULT_OK) {
                Soc_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            rtos->app->data.current = current_data;
            rtos->app->data.pack_state = pack_state;
            rtos->app->data.soc = soc_data;

            rtos->app->data.expansion.charge_remaining_time = Soc_GetChargeRemainingTime();
            rtos->app->data.expansion.discharge_remaining_time = Soc_GetDischargeRemainingTime();
            rtos->app->data.expansion.remain_capacity = soc_data.capacity;  // mAh
            rtos->app->data.expansion.remain_energy =
                (uint32_t)((float)soc_data.capacity * (float)data.voltage.pack_voltage /
                           100.0f);  // mWh

            float cycle = Soc_GetCycle();
            float soh = Soc_GetSoh();

            if (Nvs_SaveSoc(data.timestamp, soc_data.display_soc) != APP_RESULT_OK) {
                Nvs_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (Nvs_SaveCycle(cycle) != APP_RESULT_OK) {
                Nvs_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (Nvs_SaveSoh(soh) != APP_RESULT_OK) {
                Nvs_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            Rtos_SleepMs(CURRENT_READ_TICK);
            break;
        }

        case TASK_STATE_ERROR: {
            Rtos_SleepMs(500u);

            rtos->app->data.last_error_code = super.error_info.code;

            if (Current_Deinitialize() != APP_RESULT_OK) {
                Current_GetLastErrorInfo(&super.error_info);
            }

            if (Current_Initialize(&rtos->app->config) != APP_RESULT_OK) {
                Current_GetLastErrorInfo(&super.error_info);
            }

            super.state = TASK_STATE_RUN;

            break;
        }
    }
}

static void _wrapper(void* argument) {
    while (true) {
        _runner((Rtos*)argument);
    }
}

AppResult TaskCurrent_Initialize(Rtos* rtos) {
    void* task = Rtos_CreateStaticTask(_wrapper, rtos, "current", _stack, sizeof(_stack),
                                       TASK_CURRENT_PRIORITY);
    if (task == NULL) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_CURRENT_CREATE);
        return APP_RESULT_FAIL;
    }

    super.handle = task;
    super.state = TASK_STATE_RESET;
    super.error_info.code = APP_ERROR_NONE;
    super.stack_usage = 0.0f;
    super.stack = _stack;
    super.stack_size = sizeof(_stack);

    _rtos = rtos;

    rtos->task.current = &self;

    return APP_RESULT_OK;
}

AppResult TaskCurrent_Deinitialize(void) {
    if (Rtos_DeleteTask(super.handle)) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_CURRENT_DELETE);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void TaskCurrent_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = super.error_info;
    }
}
