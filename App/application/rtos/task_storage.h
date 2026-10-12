#ifndef TASK_STORAGE_H
#define TASK_STORAGE_H

#include "rtos_common.h"

AppResult TaskStorage_Initialize(Rtos* rtos);
AppResult TaskStorage_Deinitialize(void);
void TaskStorage_GetLastErrorInfo(AppErrorInfo* info);

#endif