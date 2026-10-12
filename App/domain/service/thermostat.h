#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include "../model/app_model.h"

typedef struct {
    void (*onFanStateChanged)(BmsDeviceState state);
    void (*onHeaterStateChanged)(BmsDeviceState state);
} ThermostatCallback;

AppResult Thermostat_Initialize(const BmsConfig* config, const ThermostatCallback* callback);
AppResult Thermostat_Deinitialize(void);
AppResult Thermostat_SetConfig(const BmsConfig* config);
void Thermostat_GetLastErrorInfo(AppErrorInfo* info);
void Thermostat_Process(BmsPackTemperatureData* data);

#endif