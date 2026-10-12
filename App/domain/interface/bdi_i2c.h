#ifndef BDI_I2C_H
#define BDI_I2C_H

#include "../model/app_model.h"

AppResult BdiI2c_Initialize(const BmsConfig* config);
AppResult BdiI2c_Deinitialize(void);
void BdiI2c_GetLastErrorInfo(AppErrorInfo* info);
AppResult BdiI2c_Update(const BmsData* data);

void BdiI2c_Test(const BmsData* data);

#endif