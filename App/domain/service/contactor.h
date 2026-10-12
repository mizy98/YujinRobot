#ifndef CONTACTOR_H
#define CONTACTOR_H

#include "../model/app_model.h"

typedef enum {
    CONTACTOR_TYPE_MAIN,
    CONTACTOR_TYPE_CHARGE,
    CONTACTOR_TYPE_DISCHARGE,
} ContactorType;

typedef struct {
    void (*onStateChanged)(ContactorType type, BmsContactorState state);
    void (*onError)(ContactorType type, uint8_t index, BmsContactorState expected,
                    BmsContactorState actual);
} ContactorCallback;

AppResult Contactor_Initialize(BmsConfig* config, ContactorCallback* callback);
AppResult Contactor_Deinitialize();
void Contactor_GetLastErrorInfo(AppErrorInfo* error_info);
void Contactor_SetState(ContactorType type, BmsContactorState state);
void Contactor_GetState(ContactorType type, BmsContactorState* state);
void Contactor_Process();

#endif