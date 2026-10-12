#ifndef BDI_H
#define BDI_H

#include "../model/app_model.h"

AppResult Bdi_Initialize(const BmsConfig* config);
AppResult Bdi_Deinitialize(void);
void Bdi_GetLastErrorInfo(AppErrorInfo* info);
void Bdi_SetType(BmsBdiType type);
AppResult Bdi_Process(const BmsData* data);

void Bdi_Test(const BmsData* data);

#endif