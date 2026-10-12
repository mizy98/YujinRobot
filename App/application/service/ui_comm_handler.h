#ifndef UI_COMM_HANDLER_H
#define UI_COMM_HANDLER_H

#include "../../domain/service/ui_protocol.h"
#include "../rtos/rtos_common.h"

AppResult UiCommHandler_Initialize(Rtos* rtos);
void UiCommHandler_Deinitialize();
void UiCommHandler_GetLastErrorInfo(AppErrorInfo* info);
void UiCommHandler_Process(UiProtocolCommand command, uint8_t sequence, uint8_t* payload,
                           uint16_t length);

#endif