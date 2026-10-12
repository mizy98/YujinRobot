#ifndef TASK_IDLE_H
#define TASK_IDLE_H

#include "rtos_common.h"

AppResult TaskIdle_Initialize(Rtos* rtos);
AppResult TaskIdle_Deinitialize(void);
void TaskIdle_GetLastErrorInfo(AppErrorInfo* info);

#endif