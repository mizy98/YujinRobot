#ifndef TASK_PROTECTION_H
#define TASK_PROTECTION_H

#include "rtos_common.h"

AppResult TaskProtection_Initialize(Rtos* rtos);
AppResult TaskProtection_Deinitialize(void);
void TaskProtection_GetLastErrorInfo(AppErrorInfo* info);

#endif