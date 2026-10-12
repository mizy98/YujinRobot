#ifndef TASK_RTC_H
#define TASK_RTC_H

#include "rtos_common.h"

AppResult TaskRtc_Initialize(Rtos* rtos);
AppResult TaskRtc_Deinitialize(void);
void TaskRtc_GetLastErrorInfo(AppErrorInfo* info);

#endif