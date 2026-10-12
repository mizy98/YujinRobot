#ifndef CONFIG_STORAGE_H
#define CONFIG_STORAGE_H

#include <stddef.h>

#include "../model/app_model.h"

AppResult ConfigStorage_Initialize(void);
AppResult ConfigStorage_Deinitialize(void);
void ConfigStorage_GetLastErrorInfo(AppErrorInfo* info);

AppResult ConfigStorage_Load(BmsConfig* config);
AppResult ConfigStorage_Save(const BmsConfig* config);
AppResult ConfigStorage_EraseAll(void);

#endif