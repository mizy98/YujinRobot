#include "task_external_comm.h"

#include "../../domain/interface/external_comm.h"

#define self (_task_external_comm)
#define super (_task_external_comm.super)

static TaskExternalComm _task_external_comm;
static uint8_t _stack[TASK_EXTERNAL_COMM_STACK_SIZE];
static Rtos* _rtos;

static void _runner(Rtos* rtos) {
    switch (super.state) {
        case TASK_STATE_RESET: {
            uint32_t flag = Rtos_WaitEventNoClear(rtos->event.system_ready, EVENT_SYSTEM_READY_ALL,
                                                  false, UINT32_MAX);
            if (flag == 0 || (flag & RTOS_FLAG_ERROR) != 0) {
                break;
            }

            if (ExternalComm_Initialize(rtos->app) != APP_RESULT_OK) {
                ExternalComm_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            super.state = TASK_STATE_RUN;
            break;
        }

        case TASK_STATE_RUN: {
            ExternalComm_Process();
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

AppResult TaskExternalComm_Initialize(Rtos* rtos) {
    void* task = Rtos_CreateStaticTask(_wrapper, rtos, "external_comm", _stack, sizeof(_stack),
                                       TASK_EXTERNAL_COMM_PRIORITY);
    if (task == NULL) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_EXTERNAL_COMM_CREATE);
        return APP_RESULT_FAIL;
    }

    super.handle = task;
    super.state = TASK_STATE_RESET;
    super.error_info.code = APP_ERROR_NONE;
    super.stack_usage = 0.0f;
    super.stack = _stack;
    super.stack_size = sizeof(_stack);

    _rtos = rtos;

    rtos->task.external_comm = &self;

    return APP_RESULT_OK;
}

AppResult TaskExternalComm_Deinitialize(void) {
    if (Rtos_DeleteTask(super.handle)) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_EXTERNAL_COMM_DELETE);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void TaskExternalComm_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = super.error_info;
    }
}