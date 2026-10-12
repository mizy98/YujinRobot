
#ifndef EXTERNAL_COMM_H
#define EXTERNAL_COMM_H

#include "../model/app_model.h"

AppResult ExternalComm_Initialize(App* app);
AppResult ExternalComm_Deinitialize(void);
void ExternalComm_GetLastErrorInfo(AppErrorInfo* info);
AppResult ExternalComm_Process(void);

#endif