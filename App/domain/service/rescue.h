#ifndef RESCUE_H
#define RESCUE_H

#include "../model/app_model.h"

typedef struct {
    void (*onActivated)(void);
    void (*onDeactivated)(void);
} RescueCallback;

AppResult Rescue_Initialize(const RescueCallback* callback);
AppResult Rescue_Deinitialize(void);
void Rescue_GetLastErrorInfo(AppErrorInfo* info);
void Rescue_Process(void);

#endif
