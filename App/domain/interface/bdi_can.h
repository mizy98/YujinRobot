#ifndef BDI_CAN_H
#define BDI_CAN_H

#include "../model/app_model.h"

AppResult BdiCan_Initialize(const BmsConfig* config);
AppResult BdiCan_Deinitialize(void);
void BdiCan_GetLastErrorInfo(AppErrorInfo* info);
AppResult BdiCan_Update(const BmsData* data);

#endif