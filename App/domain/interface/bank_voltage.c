#include "bank_voltage.h"

WEAK_LINK AppResult BankVoltage_Initialize(const BmsConfig* config) { return APP_RESULT_FAIL; }

WEAK_LINK AppResult BankVoltage_DeInitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void BankVoltage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        info->code = APP_ERROR_NOT_IMPLEMENT;
    }
}

WEAK_LINK AppResult BankVoltage_GetData(BmsVoltageData* data,
                                        BmsBalancerTemperatureData* temperature_data) {
    (void)data;
    (void)temperature_data;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult BankVoltage_GetInputVoltage(uint32_t* data) {
    (void)data;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult BankVoltage_GetContactorTemperatureData(BmsContactorTemperatureData* data) {
    (void)data;
    return APP_RESULT_FAIL;
}