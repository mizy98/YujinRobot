#include "rtos.h"

WEAK_LINK AppResult Rtos_Initialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK AppResult Rtos_StartScheduler(void) { return APP_RESULT_FAIL; }

WEAK_LINK uint32_t Rtos_GetTick(void) { return 0; }

WEAK_LINK void Rtos_SleepMs(uint32_t tick) {}

WEAK_LINK void Rtos_SleepUs(uint32_t us) {}

WEAK_LINK bool Rtos_IsKernelRunning(void) { return false; }

WEAK_LINK void Rtos_CreateTask(void (*task)(void*), void* argument, const char* name,
                               int32_t priority) {
    (void)task;
    (void)argument;
    (void)name;
    (void)priority;
}

WEAK_LINK void* Rtos_CreateStaticTask(void (*task)(void*), void* argument, const char* name,
                                      uint8_t* stack, uint32_t stack_size, int32_t priority) {
    (void)task;
    (void)argument;
    (void)name;
    (void)stack;
    (void)stack_size;
    (void)priority;
    return NULL;
}

WEAK_LINK AppResult Rtos_DeleteTask(void* task) {
    (void)task;
    return APP_RESULT_FAIL;
}

WEAK_LINK void* Rtos_CreateEvent(void) { return NULL; }

WEAK_LINK AppResult Rtos_DeleteEvent(void* event) {
    (void)event;
    return APP_RESULT_FAIL;
}

WEAK_LINK uint32_t Rtos_GetTaskStackSpace(void* task) {
    (void)task;
    return 0;
}

WEAK_LINK RtosTaskState Rtos_GetTaskState(void* task) {
    (void)task;
    return RTOS_TASK_STATE_INACTIVE;
}

WEAK_LINK uint32_t Rtos_WaitEvent(void* event, uint32_t flags, bool wait_any, uint32_t timeout) {
    (void)event;
    (void)flags;
    (void)wait_any;
    (void)timeout;
    return 0;
}

WEAK_LINK uint32_t Rtos_WaitEventNoClear(void* event, uint32_t flags, bool wait_any,
                                         uint32_t timeout) {
    (void)event;
    (void)flags;
    (void)wait_any;
    (void)timeout;
    return 0;
}

WEAK_LINK void Rtos_SetEvent(void* event, uint32_t flags) {
    (void)event;
    (void)flags;
}

WEAK_LINK void* Rtos_CreateQueue(uint32_t item_size, uint32_t item_count) {
    (void)item_size;
    (void)item_count;
    return NULL;
}

WEAK_LINK void* Rtos_CreateStaticQueue(uint32_t item_size, uint32_t item_count, void* memory,
                                       uint32_t memory_size) {
    (void)item_size;
    (void)item_count;
    (void)memory;
    (void)memory_size;
    return NULL;
}

WEAK_LINK AppResult Rtos_SendQueue(void* queue, const void* item, uint32_t timeout) {
    (void)queue;
    (void)item;
    (void)timeout;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult Rtos_ReceiveQueue(void* queue, void* item, uint32_t timeout) {
    (void)queue;
    (void)item;
    (void)timeout;
    return APP_RESULT_FAIL;
}

WEAK_LINK void* Rtos_CreateMutex(void) { return NULL; }

WEAK_LINK AppResult Rtos_LockMutex(void* mutex, uint32_t timeout) {
    (void)mutex;
    (void)timeout;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult Rtos_UnlockMutex(void* mutex) {
    (void)mutex;
    return APP_RESULT_FAIL;
}

WEAK_LINK void Rtos_DeleteMutex(void* mutex) { (void)mutex; }

WEAK_LINK void* Rtos_CreateTimer(void (*callback)(void*), void* argument, bool is_single_shot) {
    (void)callback;
    (void)argument;
    (void)is_single_shot;
    return NULL;
}

WEAK_LINK AppResult Rtos_DeleteTimer(void* timer) {
    (void)timer;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult Rtos_StartTimer(void* timer, uint32_t period) {
    (void)timer;
    (void)period;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult Rtos_StopTimer(void* timer) {
    (void)timer;
    return APP_RESULT_FAIL;
}