#include "../../domain/interface/rtos.h"
#include "../service/platform.h"

AppResult Rtos_Initialize(void) {
#if defined(USE_HAL_DRIVER)
    if (osKernelInitialize() != osOK) {
        return APP_RESULT_FAIL;
    }
#else
#endif
    return APP_RESULT_OK;
}

AppResult Rtos_StartScheduler(void) {
#if defined(USE_HAL_DRIVER)
    if (osKernelStart() != osOK) {
        return APP_RESULT_FAIL;
    }
#else

#endif
    return APP_RESULT_OK;
}

uint32_t Rtos_GetTick(void) {
#if defined(USE_HAL_DRIVER)
    return osKernelGetTickCount();
#else
    // TODO: implement tick count for windows
    return 0;
#endif
}

void Rtos_SleepMs(uint32_t tick) {
#if defined(USE_HAL_DRIVER)
    osDelay(tick);
#else
    // TODO: implement sleep for windows
#endif
}

void Rtos_SleepUs(uint32_t us) {
    // TODO: implement sleep us
}

bool Rtos_IsKernelRunning(void) {
#if defined(USE_HAL_DRIVER)
    return osKernelGetState() == osKernelRunning;
#else
    // TODO: implement kernel state check for windows
    return false;
#endif
}

void* Rtos_CreateStaticTask(void (*task)(void*), void* argument, const char* name, uint8_t* stack,
                            uint32_t stack_size, int32_t priority) {
#if defined(USE_HAL_DRIVER)
    if (stack_size < sizeof(StaticTask_t)) {
        return NULL;
    }

    osThreadAttr_t attribute = {0};

    attribute.name = name;
    attribute.attr_bits = osThreadDetached;
    attribute.priority = priority;
    attribute.cb_mem = stack;
    attribute.cb_size = sizeof(StaticTask_t);
    attribute.stack_mem = stack + sizeof(StaticTask_t);
    attribute.stack_size = stack_size - sizeof(StaticTask_t);

    return osThreadNew(task, argument, &attribute);
#else
    // TODO: implement task creation for windows (such as using CreateThread API)
    return NULL;
#endif
}

AppResult Rtos_DeleteTask(void* task) {
#if defined(USE_HAL_DRIVER)
    return osThreadTerminate(task) == osOK ? APP_RESULT_OK : APP_RESULT_FAIL;
#else
    // TODO: implement task delete for windows
    return APP_RESULT_FAIL;
#endif
}

uint32_t Rtos_GetTaskStackSpace(void* task) {
#if defined(USE_HAL_DRIVER)
    return osThreadGetStackSpace(task);
#else
    // TODO: implement task stack space for windows
    return 0;
#endif
}

RtosTaskState Rtos_GetTaskState(void* task) {
#if defined(USE_HAL_DRIVER)
    osThreadState_t state = osThreadGetState(task);
    if (state == osThreadInactive) {
        return RTOS_TASK_STATE_INACTIVE;
    } else if (state == osThreadReady) {
        return RTOS_TASK_STATE_READY;
    } else if (state == osThreadRunning) {
        return RTOS_TASK_STATE_RUNNING;
    } else if (state == osThreadBlocked) {
        return RTOS_TASK_STATE_BLOCKED;
    } else if (state == osThreadTerminated) {
        return RTOS_TASK_STATE_TERMINATED;
    } else {
        return RTOS_TASK_STATE_ERROR;
    }

#else
    // TODO: implement task state for windows
    return RTOS_TASK_STATE_INACTIVE;
#endif
}

void* Rtos_CreateEvent(void) {
#if defined(USE_HAL_DRIVER)
    return osEventFlagsNew(NULL);
#else
    // TODO: implement event creation for windows (such as using CreateEvent API)
    return NULL;
#endif
}

AppResult Rtos_DeleteEvent(void* event) {
#if defined(USE_HAL_DRIVER)
    if (osEventFlagsDelete(event) != osOK) {
        return APP_RESULT_FAIL;
    }
    return APP_RESULT_OK;
#else
    // TODO: implement event deletion for windows (such as using CloseHandle API)
    return APP_RESULT_FAIL;
#endif
}

uint32_t Rtos_WaitEvent(void* event, uint32_t flags, bool wait_any, uint32_t timeout) {
#if defined(USE_HAL_DRIVER)
    uint32_t rflags =
        osEventFlagsWait(event, flags, wait_any ? osFlagsWaitAny : osFlagsWaitAll, timeout);
    return rflags;
#else
    // TODO: implement event wait for windows (such as using WaitForSingleObject API)
    return 0;
#endif
}

uint32_t Rtos_WaitEventNoClear(void* event, uint32_t flags, bool wait_any, uint32_t timeout) {
#if defined(USE_HAL_DRIVER)
    return osEventFlagsWait(event, flags,
                            osFlagsNoClear | (wait_any ? osFlagsWaitAny : osFlagsWaitAll), timeout);
#else
    // TODO: implement event wait without clear for windows
    return 0;
#endif
}

void Rtos_SetEvent(void* event, uint32_t flags) {
#if defined(USE_HAL_DRIVER)
    osEventFlagsSet(event, flags);
#else
    // TODO: implement set event
#endif
}

void* Rtos_CreateQueue(uint32_t item_size, uint32_t item_count) {
#if defined(USE_HAL_DRIVER)
    return osMessageQueueNew(item_count, item_size, NULL);
#else
    // TODO: implement create queue for windows
    return NULL;
#endif
}

void* Rtos_CreateStaticQueue(uint32_t item_size, uint32_t item_count, void* memory,
                             uint32_t memory_size) {
#if defined(USE_HAL_DRIVER)
    if (memory_size < sizeof(StaticQueue_t) + (item_size * item_count)) {
        return NULL;
    }

    osMessageQueueAttr_t attribute = {
        .cb_mem = memory,
        .cb_size = sizeof(StaticQueue_t),
        .mq_mem = (uint8_t*)memory + sizeof(StaticQueue_t),
        .mq_size = item_size * item_count,
    };

    return osMessageQueueNew(item_count, item_size, &attribute);
#else
    // TODO: implement create queue for windows
    return NULL;
#endif
}

AppResult Rtos_SendQueue(void* queue, const void* item, uint32_t timeout) {
#if defined(USE_HAL_DRIVER)
    osStatus_t status = osMessageQueuePut(queue, item, 0, timeout);
    return (status == osOK) ? APP_RESULT_OK : APP_RESULT_FAIL;
#else
    // TODO: implement send queue for windows
    return APP_RESULT_FAIL;
#endif
}

AppResult Rtos_ReceiveQueue(void* queue, void* item, uint32_t timeout) {
#if defined(USE_HAL_DRIVER)
    osStatus_t status = osMessageQueueGet(queue, item, NULL, timeout);
    return (status == osOK) ? APP_RESULT_OK : APP_RESULT_FAIL;
#else
    // TODO: implement receive queue for windows
    return APP_RESULT_FAIL;
#endif
}

void* Rtos_CreateMutex(void) {
#if defined(USE_HAL_DRIVER)
    return osMutexNew(NULL);
#else
    // TODO: implement create mutex for windows
    return NULL;
#endif
}

AppResult Rtos_LockMutex(void* mutex, uint32_t timeout) {
#if defined(USE_HAL_DRIVER)
    osStatus_t status = osMutexAcquire(mutex, timeout);
    return (status == osOK) ? APP_RESULT_OK : APP_RESULT_FAIL;
#else
    // TODO: implement lock mutex for windows
    return APP_RESULT_FAIL;
#endif
}

AppResult Rtos_UnlockMutex(void* mutex) {
#if defined(USE_HAL_DRIVER)
    osStatus_t status = osMutexRelease(mutex);
    return (status == osOK) ? APP_RESULT_OK : APP_RESULT_FAIL;
#else
    // TODO: implement unlock mutex for windows
    return APP_RESULT_FAIL;
#endif
}

void Rtos_DeleteMutex(void* mutex) {
#if defined(USE_HAL_DRIVER)
    osMutexDelete(mutex);
#else
    // TODO: implement delete mutex for windows
    return APP_RESULT_FAIL;
#endif
}

void* Rtos_CreateTimer(void (*callback)(void*), void* argument, bool is_single_shot) {
#if defined(USE_HAL_DRIVER)
    return osTimerNew(callback, is_single_shot ? osTimerOnce : osTimerPeriodic, argument, NULL);
#else
    // TODO: implement create timer for windows (such as using CreateTimerQueueTimer API)
    return NULL;
#endif
}

AppResult Rtos_DeleteTimer(void* timer) {
#if defined(USE_HAL_DRIVER)
    osStatus_t status = osTimerDelete(timer);
    return (status == osOK) ? APP_RESULT_OK : APP_RESULT_FAIL;
#else
    // TODO: implement delete timer for windows
    return APP_RESULT_FAIL;
#endif
}

AppResult Rtos_StartTimer(void* timer, uint32_t period) {
#if defined(USE_HAL_DRIVER)
    return (osTimerStart(timer, period) == osOK) ? APP_RESULT_OK : APP_RESULT_FAIL;
#else
    // TODO: implement start timer for windows
    return APP_RESULT_FAIL;
#endif
}

AppResult Rtos_StopTimer(void* timer) {
#if defined(USE_HAL_DRIVER)
    return (osTimerStop(timer) == osOK) ? APP_RESULT_OK : APP_RESULT_FAIL;
#else
    // TODO: implement stop timer
    return APP_RESULT_FAIL;
#endif
}
