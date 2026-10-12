#ifndef BUZZER_H
#define BUZZER_H

#include "../model/app_model.h"

typedef enum {
    BUZZER_TYPE_NONE,
    BUZZER_TYPE_INIT_COMPLETED,    // beep short 1 time
    BUZZER_TYPE_PROTECTION,        // beep long 1 time
    BUZZER_TYPE_STORAGE_UPDATED,   // beep 1 time
    BUZZER_TYPE_VOLTAGE_ERROR,     // beep short 1 times continuously
    BUZZER_TYPE_CURRENT_ERROR,     // beep short 2 times continuously
    BUZZER_TYPE_RELAY_ERROR,       // beep short 3 times continuously
    BUZZER_TYPE_RESCUE_ACTIVATED,  // beep short 4 times continuously
} BuzzerType;

AppResult Buzzer_Initialize(void);
AppResult Buzzer_Deinitialize(void);
void Buzzer_GetLastErrorInfo(AppErrorInfo* info);
AppResult Buzzer_Beep(BuzzerType type);
AppResult Buzzer_Stop(void);
BuzzerType Buzzer_GetCurrentType(void);

#endif