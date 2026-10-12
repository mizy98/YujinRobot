#ifndef UI_COMM_UART_H
#define UI_COMM_UART_H

#include "../../domain/interface/ui_comm.h"
#include "platform.h"

AppResult UiCommUart_Initialize(const UiCommCallback* callback);
AppResult UiCommUart_Deinitialize(void);
void UiCommUart_GetLastErrorInfo(AppErrorInfo* error_info);
void UiCommUart_Process(uint32_t timeout_tick);
AppResult UiCommUart_SendData(const uint8_t* data, uint16_t size);

#endif