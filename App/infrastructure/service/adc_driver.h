#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include "../../domain/model/app_model.h"
#include "platform.h"

typedef enum {
    ADC_CHANNEL_CURRENT,
    ADC_CHANNEL_TEMPERATURE_0,
    ADC_CHANNEL_TEMPERATURE_1,
    ADC_CHANNEL_TEMPERATURE_2,
} AdcDriverChannel;

AppResult AdcDriver_Initialize(void);
AppResult AdcDriver_Deinitialize(void);
bool AdcDriver_IsInitialized(void);
void AdcDriver_GetLastErrorInfo(AppErrorInfo* info);

AppResult AdcDriver_Read(AdcDriverChannel channel, int16_t* value);
float AdcDriver_ConvertToVoltage(int16_t value);

#endif