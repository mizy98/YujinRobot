#ifndef CHARGER_H
#define CHARGER_H

#include "../model/app_model.h"

typedef struct {
    void (*onConnected)();
    void (*onDisconnected)();
    void (*onChargeStarted)();
    void (*onChargeFinished)();
} ChargerCallback;

AppResult Charger_Initialize(const BmsConfig* config, const ChargerCallback* callback);
AppResult Charger_Deinitialize(void);
void Charger_GetLastErrorInfo(AppErrorInfo* info);
AppResult Charger_Process(const BmsData* data);

#endif