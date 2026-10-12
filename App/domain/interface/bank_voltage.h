#ifndef BANK_VOLTAGE_H
#define BANK_VOLTAGE_H

#include "../model/app_model.h"

AppResult BankVoltage_Initialize(const BmsConfig* config);
AppResult BankVoltage_DeInitialize(void);
void BankVoltage_GetLastErrorInfo(AppErrorInfo* info);
AppResult BankVoltage_GetData(BmsVoltageData* data, BmsBalancerTemperatureData* temperature_data);

AppResult BankVoltage_GetInputVoltage(uint32_t* data);
AppResult BankVoltage_GetContactorTemperatureData(BmsContactorTemperatureData* data);

#endif