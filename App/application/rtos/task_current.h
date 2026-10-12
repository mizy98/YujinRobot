#ifndef TASK_CURRENT_H
#define TASK_CURRENT_H

#include "rtos_common.h"

AppResult TaskCurrent_Initialize(Rtos* rtos);
AppResult TaskCurrent_Deinitialize(void);
void TaskCurrent_GetLastErrorInfo(AppErrorInfo* info);

#endif