#include "../../domain/interface/ui_comm.h"
#include "../service/ui_comm_uart.h"
#include "../service/ui_comm_usb.h"

#define self (ui_comm_private)

CCMRAM static struct {
    bool is_usb_mounted;

    UiCommCallback callback;

    AppErrorInfo error_info;
} ui_comm_private;

static void _onUsbMountStateChanged(bool is_mounted) { self.is_usb_mounted = is_mounted; }

static void _onUsbDataReceived(const uint8_t* data, uint16_t size) {
    if (self.callback.onDataReceived != NULL) {
        self.callback.onDataReceived(data, size);
    }
}

static void _onUartDataReceived(const uint8_t* data, uint16_t size) {
    if (self.is_usb_mounted) {
        return;
    }

    if (self.callback.onDataReceived != NULL) {
        self.callback.onDataReceived(data, size);
    }
}

AppResult UiComm_Initialize(const UiCommCallback* callback) {
    if (callback == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    self.callback = *callback;

    self.is_usb_mounted = false;

    UiCommUsbCallback usb_callback = {
        .onMountStateChanged = _onUsbMountStateChanged,
        .onDtrStateChanged = NULL,
        .onDataReceived = _onUsbDataReceived,
    };

    if (UiCommUsb_Initialize(&usb_callback) != APP_RESULT_OK) {
        UiCommUsb_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    UiCommCallback uart_callback = {
        .onDataReceived = _onUartDataReceived,
    };

    if (UiCommUart_Initialize(&uart_callback) != APP_RESULT_OK) {
        UiCommUart_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    self.error_info = (AppErrorInfo){0};

    return APP_RESULT_OK;
}

AppResult UiComm_Deinitialize(void) { return APP_RESULT_OK; }

void UiComm_GetLastErrorInfo(AppErrorInfo* error_info) {
    if (error_info != NULL) {
        *error_info = self.error_info;
    }
}

void UiComm_Process(void) {
    // uint32_t timeout = self.is_usb_mounted ? UINT32_MAX : 10;

    // UiCommUsb_Process(timeout);

    // timeout = self.is_usb_mounted ? UINT32_MAX : 10;

    // if (!self.is_usb_mounted) {
    //     UiCommUart_Process(timeout);
    // }

    UiCommUsb_Process(UINT32_MAX);
}

AppResult UiComm_SendData(const uint8_t* data, uint16_t size) {
    if (self.is_usb_mounted) {
        if (UiCommUsb_SendData(data, size) != APP_RESULT_OK) {
            UiCommUsb_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }

        return APP_RESULT_OK;
    }

    if (!self.is_usb_mounted) {
        if (UiCommUart_SendData(data, size) != APP_RESULT_OK) {
            UiCommUart_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }

        return APP_RESULT_OK;
    }

    APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_INVALID_ARG);
    return APP_RESULT_FAIL;
}
