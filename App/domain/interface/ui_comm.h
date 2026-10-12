#ifndef UI_COMM_H
#define UI_COMM_H

#include "../model/app_model.h"

typedef struct {
    void (*onDataReceived)(const uint8_t* data, uint16_t size);
} UiCommCallback;

AppResult UiComm_Initialize(const UiCommCallback* callback);
AppResult UiComm_Deinitialize(void);
void UiComm_GetLastErrorInfo(AppErrorInfo* error_info);
void UiComm_Process(void);
AppResult UiComm_SendData(const uint8_t* data, uint16_t size);

#endif