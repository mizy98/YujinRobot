#ifndef TASK_EXTERNAL_COMM_H
#define TASK_EXTERNAL_COMM_H

#include "rtos_common.h"

AppResult TaskExternalComm_Initialize(Rtos* rtos);
AppResult TaskExternalComm_Deinitialize(void);
void TaskExternalComm_GetLastErrorInfo(AppErrorInfo* error_info);

#endif