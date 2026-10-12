#include "ui_comm_usb.h"

#include "tusb_adaptor.h"

#define self (ui_comm_usb_private)

CCMRAM static struct {
    bool is_mounted;
    bool is_host_dtr;
    bool is_host_rts;

    UiCommUsbCallback callback;

    AppErrorInfo error_info;
} ui_comm_usb_private;

AppResult UiCommUsb_Initialize(const UiCommUsbCallback* callback) {
    if (callback == NULL || callback->onDataReceived == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    self.callback = *callback;

    self.is_mounted = false;
    self.is_host_dtr = false;
    self.is_host_rts = false;

    self.error_info = (AppErrorInfo){0};

    if (!tusb_inited()) {
        TusbAdaptor_Initialize();
        bool result = tusb_init();
        if (!result) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_USB_DRV_INIT);
            return APP_RESULT_FAIL;
        }
    }

    return APP_RESULT_OK;
}

AppResult UiCommUsb_Deinitialize(void) { return APP_RESULT_OK; }

void UiCommUsb_GetLastErrorInfo(AppErrorInfo* error_info) {
    if (error_info != NULL) {
        *error_info = self.error_info;
    }
}

void UiCommUsb_Process(uint32_t timeout_tick) {
    tud_task_ext(timeout_tick, false);

    if (!self.is_mounted) {
        return;
    }

    if (tud_cdc_available()) {
        uint8_t buffer[CFG_TUD_CDC_RX_BUFSIZE];
        uint32_t count = tud_cdc_read(buffer, sizeof(buffer));

        self.callback.onDataReceived(buffer, (uint16_t)count);
    }
}

AppResult UiCommUsb_SendData(const uint8_t* data, uint16_t size) {
    if (!self.is_mounted) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_USB_NOT_MOUNTED);
        return APP_RESULT_FAIL;
    }

    uint16_t total_sent = 0;
    uint32_t retry_count = 0;
    const uint32_t max_retries = 100;

    while (total_sent < size) {
        uint16_t chunk_size = TU_MIN(size - total_sent, CFG_TUD_CDC_TX_BUFSIZE);
        uint32_t sent = tud_cdc_write(data + total_sent, chunk_size);

        if (sent == 0) {
            retry_count++;
            if (retry_count > max_retries) {
                APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_USB_WRITE_TIMEOUT);
                return APP_RESULT_FAIL;
            }
            tud_task_ext(10, false);
            continue;
        }

        total_sent += (uint16_t)sent;
        retry_count = 0;

        tud_cdc_write_flush();
    }

    tud_cdc_write_flush();

    return APP_RESULT_OK;
}

// =================== TinyUSB Callbacks ===================

// USB Device callback: invoked when device is mounted (configured)
void tud_mount_cb(void) {
    self.is_mounted = true;

    if (self.callback.onMountStateChanged != NULL) {
        self.callback.onMountStateChanged(self.is_mounted);
    }
}

// USB Device callback: invoked when device is unmounted
void tud_umount_cb(void) {
    self.is_mounted = false;

    if (self.callback.onMountStateChanged != NULL) {
        self.callback.onMountStateChanged(self.is_mounted);
    }
}

// USB Device callback: invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    self.is_mounted = false;

    if (self.callback.onMountStateChanged != NULL) {
        self.callback.onMountStateChanged(self.is_mounted);
    }
}

// USB Device callback: invoked when usb bus is resumed
void tud_resume_cb(void) {
    self.is_mounted = true;

    if (self.callback.onMountStateChanged != NULL) {
        self.callback.onMountStateChanged(self.is_mounted);
    }
}

// USB Device CDC callback: when the host opens/closes the connection
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    (void)itf;
    self.is_host_dtr = dtr;
    self.is_host_rts = rts;

    if (self.callback.onDtrStateChanged != NULL) {
        self.callback.onDtrStateChanged(dtr);
    }
}
