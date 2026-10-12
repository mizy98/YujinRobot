#ifndef ERROR_LOG_STORAGE_H
#define ERROR_LOG_STORAGE_H

#include "../model/app_model.h"

AppResult ErrorLogStorage_Initialize();
AppResult ErrorLogStorage_Deinitialize();
void ErrorLogStorage_GetLastErrorInfo(AppErrorInfo* info);

AppResult ErrorLogStorage_Record(const AppErrorInfo* error_info);

#endif