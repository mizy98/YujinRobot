#ifndef CURRENT_H
#define CURRENT_H

#include "../model/app_model.h"

AppResult Current_Initialize(const BmsConfig* config);
AppResult Current_Deinitialize(void);
void Current_GetLastErrorInfo(AppErrorInfo* info);
AppResult Current_GetData(BmsPackState pack_state, BmsCurrentData* data);

#endif