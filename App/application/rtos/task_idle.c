#include "task_idle.h"

#include "../../domain/interface/signal.h"
#include "../../domain/service/buzzer.h"
#include "../../domain/service/indicator.h"
#include "../service/rtos_status.h"

#define self (_task_idle)
#define super (_task_idle.super)

APP_CCMRAM static TaskIdle _task_idle;
APP_CCMRAM static uint8_t _stack[TASK_IDLE_STACK_SIZE];
APP_CCMRAM static Rtos* _rtos;

static void _runner(Rtos* rtos) {
    switch (super.state) {
        case TASK_STATE_RESET: {
            uint32_t flag = Rtos_WaitEventNoClear(rtos->event.system_ready, EVENT_SYSTEM_READY_ALL,
                                                  false, UINT32_MAX);
            if (flag == 0 || (flag & RTOS_FLAG_ERROR) != 0) {
                break;
            }

            if (Buzzer_Initialize() != APP_RESULT_OK) {
                Buzzer_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            Indicator_Initialize();

            Buzzer_Beep(BUZZER_TYPE_INIT_COMPLETED);

            super.state = TASK_STATE_RUN;
            break;
        }

        case TASK_STATE_RUN: {
            RtosStatus_Process(rtos);

            Rtos_SleepMs(500u);
            break;
        }

        case TASK_STATE_ERROR: {
            Rtos_SleepMs(1000);

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

AppResult TaskIdle_Initialize(Rtos* rtos) {
    void* task =
        Rtos_CreateStaticTask(_wrapper, rtos, "idle", _stack, sizeof(_stack), TASK_IDLE_PRIORITY);
    if (task == NULL) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_IDLE_CREATE);
        return APP_RESULT_FAIL;
    }

    super.handle = task;
    super.state = TASK_STATE_RESET;
    super.error_info.code = APP_ERROR_NONE;
    super.stack_usage = 0.0f;
    super.stack = _stack;
    super.stack_size = sizeof(_stack);

    _rtos = rtos;

    rtos->task.idle = &self;

    return APP_RESULT_OK;
}

AppResult TaskIdle_Deinitialize(void) {
    if (Rtos_DeleteTask(super.handle)) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_IDLE_DELETE);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void TaskIdle_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = super.error_info;
    }
}
