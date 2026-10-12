#include "rtos_status.h"

#define self (rtos_status)

#define STACK_USAGE_CALCULATION_TICK (1000u * 60u)
#define STACK_USAGE_CHECK_TASK_STATE_TICK (1000u * 30u)

static struct {
    struct {
        uint32_t last_calculation_tick;  // ms
    } stack_usage;

    struct {
        uint32_t last_check_tick;  // ms
    } state;
} rtos_status;

void _calculateStackUsage(SuperTask* task) {
    if (task == NULL || task->stack_size == 0) {
        return;
    }

    uint32_t stack_space = Rtos_GetTaskStackSpace(task->handle);
    if (stack_space > task->stack_size) {
        stack_space = task->stack_size;
    }

    task->stack_usage =
        ((float)(task->stack_size - stack_space) / (float)task->stack_size) * 100.0f;
}

void _checkTaskState(SuperTask* task) {
    if (task == NULL) {
        return;
    }

    task->rtos_state = Rtos_GetTaskState(task->handle);
}

void RtosStatus_Process(Rtos* rtos) {
    uint32_t current_tick = Rtos_GetTick();

    if ((current_tick - self.stack_usage.last_calculation_tick) >= STACK_USAGE_CALCULATION_TICK) {
        _calculateStackUsage(&rtos->task.idle->super);
        _calculateStackUsage(&rtos->task.storage->super);
        _calculateStackUsage(&rtos->task.current->super);
        _calculateStackUsage(&rtos->task.voltage->super);
        _calculateStackUsage(&rtos->task.temperature->super);
        _calculateStackUsage(&rtos->task.protection->super);
        _calculateStackUsage(&rtos->task.external_comm->super);
        _calculateStackUsage(&rtos->task.ui_comm->super);

        self.stack_usage.last_calculation_tick = current_tick;
    }

    if ((current_tick - self.state.last_check_tick) >= STACK_USAGE_CHECK_TASK_STATE_TICK) {
        _checkTaskState(&rtos->task.idle->super);
        _checkTaskState(&rtos->task.storage->super);
        _checkTaskState(&rtos->task.current->super);
        _checkTaskState(&rtos->task.voltage->super);
        _checkTaskState(&rtos->task.temperature->super);
        _checkTaskState(&rtos->task.protection->super);
        _checkTaskState(&rtos->task.external_comm->super);
        _checkTaskState(&rtos->task.ui_comm->super);

        self.state.last_check_tick = current_tick;
    }
}
