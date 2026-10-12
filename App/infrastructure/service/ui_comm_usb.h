#ifndef UI_COMM_USB_H
#define UI_COMM_USB_H

#include "../../domain/interface/ui_comm.h"
#include "platform.h"

typedef struct {
    void (*onMountStateChanged)(bool is_mounted);
    void (*onDtrStateChanged)(bool is_enabled);
    void (*onDataReceived)(const uint8_t* data, uint16_t size);
} UiCommUsbCallback;

AppResult UiCommUsb_Initialize(const UiCommUsbCallback* callback);
AppResult UiCommUsb_Deinitialize(void);
void UiCommUsb_GetLastErrorInfo(AppErrorInfo* error_info);
void UiCommUsb_Process(uint32_t timeout_tick);
AppResult UiCommUsb_SendData(const uint8_t* data, uint16_t size);

#endif