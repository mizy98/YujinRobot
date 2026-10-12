#ifndef NTC_THERMISTOR_H
#define NTC_THERMISTOR_H

#include "../model/app_model.h"

typedef enum {
    NTC_THERMISTOR_TYPE_10K_3950,
    NTC_THERMISTOR_TYPE_10K_3630,
} NtcThermistorType;

void NtcThermistor_Initialize(float serial_resistor, uint32_t adc_resolution);
float NtcThermistor_GetTemperature(NtcThermistorType type, uint16_t adc);

#endif