#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include <stddef.h>
#include <stdint.h>

#include "../model/app_model.h"

AppResult SystemTime_Initialize(void);
AppResult SystemTime_Deinitialize(void);
void SystemTime_GetLastErrorInfo(AppErrorInfo* info);
AppResult SystemTime_SetTimestamp(uint64_t timestamp);
AppResult SystemTime_GetTimestamp(uint64_t* timestamp);
uint32_t SystemTime_GetTick(void);

#endif