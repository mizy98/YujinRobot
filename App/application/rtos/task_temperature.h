#ifndef TASK_TEMPERATURE_H
#define TASK_TEMPERATURE_H

#include "rtos_common.h"

AppResult TaskTemperature_Initialize(Rtos* rtos);
AppResult TaskTemperature_Deinitialize(void);
void TaskTemperature_GetLastErrorInfo(AppErrorInfo* error_info);

#endif