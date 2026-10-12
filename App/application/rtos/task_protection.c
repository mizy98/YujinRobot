#include "task_protection.h"

#include "../../domain/service/buzzer.h"
#include "../../domain/service/contactor.h"
#include "../../domain/service/nvs.h"
#include "../../domain/service/protection.h"
#include "../../domain/service/rescue.h"

#define self (_task_protection)
#define super (_task_protection.super)

APP_CCMRAM static TaskProtection _task_protection;
APP_CCMRAM static uint8_t _stack[TASK_PROTECTION_STACK_SIZE];
APP_CCMRAM static Rtos* _rtos;

// called in task context
static void _onProtectionOccurred(BmsProtectionCode code, int32_t value, int32_t threshold) {
    BmsProtectionState state = _rtos->app->data.protection_state;
    state.value |= (1 << code);
    _rtos->app->data.protection_state = state;

    self.protection_snapshot_value = value;
    self.protection_snapshot_threshold = threshold;

    Rtos_SetEvent(_rtos->event.contactor, EVENT_CONTACTOR_ALL_OFF);

    Buzzer_Beep(BUZZER_TYPE_PROTECTION);
}

// called in task context
static void _onProtectionReleased(BmsProtectionCode code, int32_t value, int32_t threshold) {
    BmsProtectionState state = _rtos->app->data.protection_state;
    state.value &= ~(1 << code);
    _rtos->app->data.protection_state = state;

    self.protection_snapshot_value = value;
    self.protection_snapshot_threshold = threshold;

    if (state.value == 0) {
        Rtos_SetEvent(_rtos->event.contactor, EVENT_CONTACTOR_ALL_ON);
    }

    if (Buzzer_GetCurrentType() == BUZZER_TYPE_PROTECTION) {
        Buzzer_Stop();
    }
}

// called in task context
static void _onPermanentFailure(BmsProtectionCode code, int32_t value, int32_t threshold) {
    BmsProtectionState protection_state = _rtos->app->data.protection_state;
    BmsProtectionState pf_state = _rtos->app->data.pf_state;

    protection_state.value |= (1 << code);
    pf_state.value |= (1 << code);

    _rtos->app->data.protection_state = protection_state;
    _rtos->app->data.pf_state = pf_state;

    Rtos_SetEvent(_rtos->event.contactor, EVENT_CONTACTOR_ALL_OFF);

    Buzzer_Beep(BUZZER_TYPE_PROTECTION);

    if (Nvs_SavePfState(pf_state.value) != APP_RESULT_OK) {
        Nvs_GetLastErrorInfo(&super.error_info);
    }
}

// called in task context
static void _onPermanentFailureReleased(BmsProtectionCode code) {
    BmsProtectionState protection_state = _rtos->app->data.protection_state;
    BmsProtectionState pf_state = _rtos->app->data.pf_state;

    protection_state.value &= ~(1 << code);
    pf_state.value &= ~(1 << code);

    _rtos->app->data.protection_state = protection_state;
    _rtos->app->data.pf_state = pf_state;

    if (protection_state.value == 0) {
        Rtos_SetEvent(_rtos->event.contactor, EVENT_CONTACTOR_ALL_ON);
    }

    if (Buzzer_GetCurrentType() == BUZZER_TYPE_PROTECTION) {
        Buzzer_Stop();
    }

    if (Nvs_SavePfState(pf_state.value) != APP_RESULT_OK) {
        Nvs_GetLastErrorInfo(&super.error_info);
    }
}

// called in task context
static void _onContactorStateChanged(ContactorType type, BmsContactorState state) {
    switch (type) {
        case CONTACTOR_TYPE_MAIN:
            _rtos->app->data.main_contactor_state = state;
            break;
        case CONTACTOR_TYPE_CHARGE:
            _rtos->app->data.charge_contactor_state = state;
            break;
        case CONTACTOR_TYPE_DISCHARGE:
            _rtos->app->data.discharge_contactor_state = state;
            break;
    }
}

// called in task context
static void _onContactorError(ContactorType type, uint8_t index, BmsContactorState expected,
                              BmsContactorState actual) {
    switch (type) {
        case CONTACTOR_TYPE_MAIN:
            _rtos->app->data.main_contactor_state = BMS_DEVICE_STATE_ERROR;
            break;
        case CONTACTOR_TYPE_CHARGE:
            _rtos->app->data.charge_contactor_state = BMS_DEVICE_STATE_ERROR;
            break;
        case CONTACTOR_TYPE_DISCHARGE:
            _rtos->app->data.discharge_contactor_state = BMS_DEVICE_STATE_ERROR;
            break;
    }

    Contactor_SetState(CONTACTOR_TYPE_CHARGE, BMS_CONTACTOR_STATE_OFF);
    Contactor_SetState(CONTACTOR_TYPE_DISCHARGE, BMS_CONTACTOR_STATE_OFF);

    Buzzer_Beep(BUZZER_TYPE_PROTECTION);
}

static void _onRescueActivated(void) {
    if (_rtos->app->data.protection_state.uvp) {
        Contactor_SetState(CONTACTOR_TYPE_CHARGE, BMS_CONTACTOR_STATE_ON);
        Contactor_SetState(CONTACTOR_TYPE_DISCHARGE, BMS_CONTACTOR_STATE_ON);
    }

    _rtos->app->config.protection.uvp.is_enabled = 0;
    Protection_UpdateConfig(&_rtos->app->config);

    Buzzer_Beep(BUZZER_TYPE_RESCUE_ACTIVATED);
}

static void _onRescueDeactivated(void) {
    _rtos->app->config.protection.uvp.is_enabled = 1;
    Protection_UpdateConfig(&_rtos->app->config);

    if (Buzzer_GetCurrentType() == BUZZER_TYPE_RESCUE_ACTIVATED) {
        Buzzer_Stop();
    }
}

static void _runner(Rtos* rtos) {
    switch (super.state) {
        case TASK_STATE_RESET: {
            uint32_t flag = EVENT_SYSTEM_READY_STORAGE | EVENT_SYSTEM_READY_VOLTAGE |
                            EVENT_SYSTEM_READY_TEMPERATURE | EVENT_SYSTEM_READY_CURRENT;

            flag = Rtos_WaitEventNoClear(rtos->event.system_ready, flag, false, UINT32_MAX);
            if (((flag & (EVENT_SYSTEM_READY_STORAGE | EVENT_SYSTEM_READY_VOLTAGE |
                          EVENT_SYSTEM_READY_TEMPERATURE | EVENT_SYSTEM_READY_CURRENT)) == 0) ||
                ((flag & RTOS_FLAG_ERROR) != 0)) {
                APP_ERROR_SET_INFO(super.error_info,
                                   APP_ERROR_TASK_PROTECTION_SYSTEM_READY_TIMEOUT);
                super.state = TASK_STATE_ERROR;
                break;
            }

            BmsProtectionState pf_state = {0};
            // if (Nvs_LoadPfState(&pf_state.value) != APP_RESULT_OK) {
            //     Nvs_GetLastErrorInfo(&super.error_info);
            //     super.state = TASK_STATE_ERROR;
            //     break;
            // }

            // if (pf_state.value == 0xFFFFFFFF) {
            //     pf_state.value = 0;
            // }

            // rtos->app->data.pf_state = pf_state;

            if (Protection_Initialize(&rtos->app->config, pf_state,
                                      &(ProtectionCallback){
                                          .onProtectionOccurred = _onProtectionOccurred,
                                          .onProtectionReleased = _onProtectionReleased,
                                          .onPermanentFailure = _onPermanentFailure,
                                          .onPermanentFailureReleased = _onPermanentFailureReleased,
                                      }) != APP_RESULT_OK) {
                Protection_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (Contactor_Initialize(&rtos->app->config,
                                     &(ContactorCallback){
                                         .onStateChanged = _onContactorStateChanged,
                                         .onError = _onContactorError,
                                     }) != APP_RESULT_OK) {
                Contactor_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (Rescue_Initialize(&(RescueCallback){
                    .onActivated = _onRescueActivated,
                    .onDeactivated = _onRescueDeactivated,
                }) != APP_RESULT_OK) {
                Rescue_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            Rtos_SetEvent(rtos->event.contactor,
                          EVENT_CONTACTOR_CHARGE_ON | EVENT_CONTACTOR_DISCHARGE_ON);

            super.state = TASK_STATE_RUN;
            break;
        }

        case TASK_STATE_RUN: {
            uint32_t flag = EVENT_CONTACTOR_MAIN_ON | EVENT_CONTACTOR_MAIN_OFF |
                            EVENT_CONTACTOR_CHARGE_ON | EVENT_CONTACTOR_CHARGE_OFF |
                            EVENT_CONTACTOR_DISCHARGE_ON | EVENT_CONTACTOR_DISCHARGE_OFF |
                            EVENT_CONTACTOR_ALL_ON | EVENT_CONTACTOR_ALL_OFF;
            flag = Rtos_WaitEvent(rtos->event.contactor, flag, true, 0);
            if (flag & RTOS_FLAG_ERROR) {
                APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_PROTECTION_CONTACTOR_EVENT);
            } else {
                bool has_all_off = (flag & EVENT_CONTACTOR_ALL_OFF) != 0;
                bool has_all_on = (flag & EVENT_CONTACTOR_ALL_ON) != 0;

                bool main_off = has_all_off || ((flag & EVENT_CONTACTOR_MAIN_OFF) != 0);
                bool main_on = has_all_on || ((flag & EVENT_CONTACTOR_MAIN_ON) != 0);
                bool charge_off = has_all_off || ((flag & EVENT_CONTACTOR_CHARGE_OFF) != 0);
                bool charge_on = has_all_on || ((flag & EVENT_CONTACTOR_CHARGE_ON) != 0);
                bool discharge_off = has_all_off || ((flag & EVENT_CONTACTOR_DISCHARGE_OFF) != 0);
                bool discharge_on = has_all_on || ((flag & EVENT_CONTACTOR_DISCHARGE_ON) != 0);

                // OFF has priority when ON/OFF requests collide in the same cycle.
                if (main_off) {
                    Contactor_SetState(CONTACTOR_TYPE_MAIN, BMS_CONTACTOR_STATE_OFF);
                } else if (main_on) {
                    Contactor_SetState(CONTACTOR_TYPE_MAIN, BMS_CONTACTOR_STATE_ON);
                }

                if (charge_off) {
                    Contactor_SetState(CONTACTOR_TYPE_CHARGE, BMS_CONTACTOR_STATE_OFF);
                } else if (charge_on) {
                    Contactor_SetState(CONTACTOR_TYPE_CHARGE, BMS_CONTACTOR_STATE_ON);
                }

                if (discharge_off) {
                    Contactor_SetState(CONTACTOR_TYPE_DISCHARGE, BMS_CONTACTOR_STATE_OFF);
                } else if (discharge_on) {
                    Contactor_SetState(CONTACTOR_TYPE_DISCHARGE, BMS_CONTACTOR_STATE_ON);
                }
            }

            flag = Rtos_WaitEvent(rtos->event.protection, EVENT_PROTECTION_UPDATE, true, 0);
            if (flag & RTOS_FLAG_ERROR) {
                APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_PROTECTION_UPDATE_CONFIG_EVENT);

            } else if (flag & EVENT_PROTECTION_UPDATE) {
                Protection_UpdateConfig(&rtos->app->config);
            }

            BmsData data = rtos->app->data;

            Protection_Check(&data);

            Rescue_Process();

            Contactor_Process();

            Rtos_SleepMs(PROTECTION_CHECK_TICK);
            break;
        }

        case TASK_STATE_ERROR: {
            Rtos_SleepMs(1000);
            break;
        }
    }
}

static void _wrapper(void* argument) {
    while (true) {
        _runner((Rtos*)argument);
    }
}

AppResult TaskProtection_Initialize(Rtos* rtos) {
    void* task = Rtos_CreateStaticTask(_wrapper, rtos, "protection", _stack, sizeof(_stack),
                                       TASK_PROTECTION_PRIORITY);
    if (task == NULL) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_PROTECTION_CREATE);
        return APP_RESULT_FAIL;
    }

    super.handle = task;
    super.state = TASK_STATE_RESET;
    super.error_info.code = APP_ERROR_NONE;
    super.stack_usage = 0.0f;
    super.stack = _stack;
    super.stack_size = sizeof(_stack);

    _rtos = rtos;

    rtos->task.protection = &self;

    return APP_RESULT_OK;
}

AppResult TaskProtection_Deinitialize(void) {
    if (Rtos_DeleteTask(super.handle)) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_PROTECTION_DELETE);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void TaskProtection_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = super.error_info;
    }
}
