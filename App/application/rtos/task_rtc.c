#include "task_rtc.h"

#include "../../domain/interface/system_time.h"
#include "../../domain/service/bdi.h"
#include "../../domain/service/buzzer.h"
#include "../../domain/service/nvs.h"

#define self (_task_rtc)
#define super (_task_rtc.super)

APP_CCMRAM static TaskRtc _task_rtc;
APP_CCMRAM static uint8_t _stack[TASK_RTC_STACK_SIZE];
APP_CCMRAM static Rtos* _rtos;

static void _runner(Rtos* rtos) {
    switch (super.state) {
        case TASK_STATE_RESET: {
            if (SystemTime_Initialize() != APP_RESULT_OK) {
                SystemTime_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (Nvs_Initialize() != APP_RESULT_OK) {
                Nvs_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (Bdi_Initialize(&rtos->app->config) != APP_RESULT_OK) {
                Bdi_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            uint64_t timestamp = 0;
            if (SystemTime_GetTimestamp(&timestamp) != APP_RESULT_OK) {
                SystemTime_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            rtos->app->data.timestamp = (uint32_t)timestamp;

            if (Nvs_IsFormatOccurred()) {
                rtos->app->boot_info.is_backup_valid = false;
            } else {
                rtos->app->boot_info.is_backup_valid = true;
            }

            Rtos_SetEvent(rtos->event.system_ready, EVENT_SYSTEM_READY_RTC);

            super.state = TASK_STATE_RUN;
            break;
        }

        case TASK_STATE_RUN: {
            uint64_t timestamp = 0;
            if (SystemTime_GetTimestamp(&timestamp) != APP_RESULT_OK) {
                SystemTime_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            uint32_t last_timestamp = rtos->app->data.timestamp;
            if (((uint32_t)timestamp != last_timestamp) && (timestamp != 0)) {
                rtos->app->data.timestamp = (uint32_t)timestamp;

                Rtos_SetEvent(rtos->event.storage, EVENT_STORAGE_RECORD_DATA);
            }

            if (Bdi_Process(&rtos->app->data) != APP_RESULT_OK) {
                Bdi_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            Rtos_SleepMs(500u);
            break;
        }

        case TASK_STATE_ERROR: {
            if (SystemTime_Deinitialize() != APP_RESULT_OK) {
                SystemTime_GetLastErrorInfo(&super.error_info);
            }

            if (Nvs_Deinitialize() != APP_RESULT_OK) {
                Nvs_GetLastErrorInfo(&super.error_info);
            }

            if (Bdi_Deinitialize() != APP_RESULT_OK) {
                Bdi_GetLastErrorInfo(&super.error_info);
            }

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

AppResult TaskRtc_Initialize(Rtos* rtos) {
    void* task =
        Rtos_CreateStaticTask(_wrapper, rtos, "rtc", _stack, sizeof(_stack), TASK_RTC_PRIORITY);
    if (task == NULL) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_RTC_CREATE);
        return APP_RESULT_FAIL;
    }

    super.handle = task;
    super.state = TASK_STATE_RESET;
    super.error_info.code = APP_ERROR_NONE;
    super.stack_usage = 0.0f;
    super.stack = _stack;
    super.stack_size = sizeof(_stack);

    _rtos = rtos;

    rtos->task.rtc = &self;

    return APP_RESULT_OK;
}

AppResult TaskRtc_Deinitialize(void) {
    if (Rtos_DeleteTask(super.handle)) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_RTC_DELETE);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void TaskRtc_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = super.error_info;
    }
}
