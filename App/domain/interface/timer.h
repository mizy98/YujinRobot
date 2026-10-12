#ifndef TIMER_H
#define TIMER_H

#include "../model/app_model.h"

AppResult Timer_Initialize(void);
AppResult Timer_Deinitialize(void);
void Timer_GetLastErrorInfo(AppErrorInfo* info);

void* Timer_Create(void (*callback)(void* context), bool is_singleshot, void* context);
AppResult Timer_Dispose(void* instance);
AppResult Timer_Start(void* instance, uint32_t tick, bool is_singleshot, void* context);
AppResult Timer_Stop(void* instance);
bool Timer_IsRunning(void* instance);

#endif