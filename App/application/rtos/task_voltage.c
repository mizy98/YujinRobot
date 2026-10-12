#include "task_voltage.h"

#include "../../domain/interface/bank_voltage.h"

#define self (_task_voltage)
#define super (_task_voltage.super)

static TaskVoltage _task_voltage;
static uint8_t _stack[TASK_VOLTAGE_STACK_SIZE];

static void _runner(Rtos* rtos) {
    switch (super.state) {
        case TASK_STATE_RESET: {
            uint32_t flag = EVENT_SYSTEM_READY_STORAGE;

            flag = Rtos_WaitEventNoClear(rtos->event.system_ready, flag, false, UINT32_MAX);
            if (((flag & EVENT_SYSTEM_READY_STORAGE) == 0) || ((flag & RTOS_FLAG_ERROR) != 0)) {
                APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_VOLTAGE_SYSTEM_READY_TIMEOUT);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (BankVoltage_Initialize(&rtos->app->config) != APP_RESULT_OK) {
                BankVoltage_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
            }

            BmsVoltageData voltage_data = {0};
            BmsBalancerTemperatureData temperature_data = {0};
            if (BankVoltage_GetData(&voltage_data, &temperature_data) != APP_RESULT_OK) {
                BankVoltage_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            rtos->app->data.voltage = voltage_data;
            rtos->app->data.balancer_temperature = temperature_data;

            Rtos_SetEvent(rtos->event.system_ready, EVENT_SYSTEM_READY_VOLTAGE);

            super.state = TASK_STATE_RUN;
            break;
        }

        case TASK_STATE_RUN: {
            BmsVoltageData voltage_data = {0};
            BmsBalancerTemperatureData temperature_data = {0};
            BmsContactorTemperatureData contactor_temperature_data = {0};
            uint32_t input_voltage = 0;

            if (BankVoltage_GetData(&voltage_data, &temperature_data) != APP_RESULT_OK) {
                BankVoltage_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (BankVoltage_GetContactorTemperatureData(&contactor_temperature_data) !=
                APP_RESULT_OK) {
                BankVoltage_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            if (BankVoltage_GetInputVoltage(&input_voltage) != APP_RESULT_OK) {
                BankVoltage_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            rtos->app->data.voltage = voltage_data;
            rtos->app->data.balancer_temperature = temperature_data;
            rtos->app->data.contactor_temperature = contactor_temperature_data;

            rtos->app->data.expansion.input_voltage = input_voltage;

            Rtos_SleepMs(VOLTAGE_READ_TICK);
            break;
        }

        case TASK_STATE_ERROR: {
            (void)BankVoltage_DeInitialize();
            Rtos_SleepMs(1000);
            super.state = TASK_STATE_RESET;
            break;
        }
    }
}

static void _wrapper(void* argument) {
    while (true) {
        _runner((Rtos*)argument);
    }
}

AppResult TaskVoltage_Initialize(Rtos* rtos) {
    void* task = Rtos_CreateStaticTask(_wrapper, rtos, "voltage", _stack, sizeof(_stack),
                                       TASK_VOLTAGE_PRIORITY);
    if (task == NULL) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_VOLTAGE_CREATE);
        return APP_RESULT_FAIL;
    }

    super.handle = task;
    super.state = TASK_STATE_RESET;
    super.error_info.code = APP_ERROR_NONE;
    super.stack_usage = 0.0f;
    super.stack = _stack;
    super.stack_size = sizeof(_stack);

    rtos->task.voltage = &self;

    return APP_RESULT_OK;
}

AppResult TaskVoltage_Deinitialize(void) {
    if (Rtos_DeleteTask(super.handle)) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_VOLTAGE_DELETE);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void TaskVoltage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = super.error_info;
    }
}
