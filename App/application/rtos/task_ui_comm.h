#ifndef TASK_UI_COMM_H
#define TASK_UI_COMM_H

#include "rtos_common.h"

AppResult TaskUiComm_Initialize(Rtos* rtos);
AppResult TaskUiComm_Deinitialize(void);
void TaskUiComm_GetLastErrorInfo(AppErrorInfo* error_info);

#endif