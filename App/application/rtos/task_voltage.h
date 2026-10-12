#ifndef TASK_VOLTAGE_H
#define TASK_VOLTAGE_H

#include "rtos_common.h"

AppResult TaskVoltage_Initialize(Rtos* rtos);
AppResult TaskVoltage_Deinitialize(void);
void TaskVoltage_GetLastErrorInfo(AppErrorInfo* info);

#endif