#ifndef UI_PROTOCOL_H
#define UI_PROTOCOL_H

#include "../model/app_model.h"

typedef enum {
    UI_PROTOCOL_RESET,
    UI_PROTOCOL_GET_SIZE_INFO,
    UI_PROTOCOL_GET_INFO,
    UI_PROTOCOL_SET_CONFIG,
    UI_PROTOCOL_GET_CONFIG,
    UI_PROTOCOL_GET_DATA,
    UI_PROTOCOL_GET_RTOS_STATUS,
    UI_PROTOCOL_START_DATA_STREAM,
    UI_PROTOCOL_STOP_DATA_STREAM,
    UI_PROTOCOL_START_BRIEF_DATA_STREAM,
    UI_PROTOCOL_STOP_BRIEF_DATA_STREAM,
    UI_PROTOCOL_GET_DATA_LOG_ENTRY,
    UI_PROTOCOL_GET_DATA_LOG_INFO,
    UI_PROTOCOL_GET_DATA_LOG_CHUNK,
    UI_PROTOCOL_RESET_DATA_LOG,
    UI_PROTOCOL_GET_EVENT_LOG_ENTRY,
    UI_PROTOCOL_GET_EVENT_LOG_INFO,
    UI_PROTOCOL_GET_EVENT_LOG_CHUNK,
    UI_PROTOCOL_RESET_EVENT_LOG,
    UI_PROTOCOL_SET_TIMESTAMP = 0x50,
} UiProtocolCommand;

typedef struct {
    void (*onCommandReceived)(UiProtocolCommand command, uint8_t sequence, uint8_t* payload,
                              uint16_t length);
} UiProtocolCallback;

AppResult UiProtocol_Initialize(UiProtocolCallback* callback);
AppResult UiProtocol_Deinitialize();
void UiProtocol_GetLastErrorInfo(AppErrorInfo* error_info);
void UiProtocol_Process();
void UiProtocol_Parse();
AppResult UiProtocol_SendDataWithSequence(UiProtocolCommand command, uint8_t sequence,
                                          const uint8_t* data, uint16_t size);
AppResult UiProtocol_SendData(UiProtocolCommand command, const uint8_t* data, uint16_t size);

#endif