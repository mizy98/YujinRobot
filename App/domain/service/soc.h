#ifndef SOC_H
#define SOC_H

#include "../model/app_model.h"

AppResult Soc_Initialize(BmsConfig* config, uint16_t previous_soc, uint16_t previous_cycle,
                         uint16_t previous_soh, int16_t ocv, int16_t temperature,
                         bool is_force_calibration);
void Soc_GetLastErrorInfo(AppErrorInfo* info);

AppResult Soc_Calculate(BmsData* data, BmsSocData* soc_data);
float Soc_GetCycle();
float Soc_GetSoh();
uint32_t Soc_GetChargeRemainingTime();
uint32_t Soc_GetDischargeRemainingTime();

#endif