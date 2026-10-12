#ifndef RTOS_H
#define RTOS_H

#include "../model/app_model.h"

#define RTOS_FLAG_TIMEOUT (0xFFFFFFFEU)
#define RTOS_FLAG_ERROR (0x80000000U)

typedef enum {
    RTOS_TASK_STATE_INACTIVE = 0,
    RTOS_TASK_STATE_READY,
    RTOS_TASK_STATE_RUNNING,
    RTOS_TASK_STATE_BLOCKED,
    RTOS_TASK_STATE_TERMINATED,
    RTOS_TASK_STATE_ERROR,
} RtosTaskState;

AppResult Rtos_Initialize(void);
AppResult Rtos_StartScheduler(void);
uint32_t Rtos_GetTick(void);

void Rtos_SleepMs(uint32_t tick);
void Rtos_SleepUs(uint32_t us);

void Rtos_CreateTask(void (*task)(void*), void* argument, const char* name, int32_t priority);
void* Rtos_CreateStaticTask(void (*task)(void*), void* argument, const char* name, uint8_t* stack,
                            uint32_t stack_size, int32_t priority);
AppResult Rtos_DeleteTask(void* task);
uint32_t Rtos_GetTaskStackSpace(void* task);
RtosTaskState Rtos_GetTaskState(void* task);

void* Rtos_CreateEvent(void);
AppResult Rtos_DeleteEvent(void* event);
uint32_t Rtos_WaitEvent(void* event, uint32_t flags, bool wait_any, uint32_t timeout);
uint32_t Rtos_WaitEventNoClear(void* event, uint32_t flags, bool wait_any, uint32_t timeout);
void Rtos_SetEvent(void* event, uint32_t flags);

void* Rtos_CreateQueue(uint32_t item_size, uint32_t item_count);
void* Rtos_CreateStaticQueue(uint32_t item_size, uint32_t item_count, void* memory,
                             uint32_t memory_size);
AppResult Rtos_SendQueue(void* queue, const void* item, uint32_t timeout);
AppResult Rtos_ReceiveQueue(void* queue, void* item, uint32_t timeout);

void* Rtos_CreateMutex(void);
AppResult Rtos_LockMutex(void* mutex, uint32_t timeout);
AppResult Rtos_UnlockMutex(void* mutex);
void Rtos_DeleteMutex(void* mutex);

void* Rtos_CreateTimer(void (*callback)(void*), void* argument, bool is_single_shot);
AppResult Rtos_DeleteTimer(void* timer);
AppResult Rtos_StartTimer(void* timer, uint32_t period);
AppResult Rtos_StopTimer(void* timer);

#endif