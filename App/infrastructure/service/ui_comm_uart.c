#include "ui_comm_uart.h"

#define self (ui_comm_uart_private)

#define UART_RX_BUFFER_SIZE (2048u)
#define UART_TX_BITS_PER_BYTE (10u)  // start + 8 data + stop, no parity
#define UART_TX_TIMEOUT_MARGIN_MS (20u)

typedef enum {
    EVENT_UART_TX_COMPLETE = (1 << 0),
    EVENT_UART_RX_COMPLETE = (1 << 1),
    EVENT_UART_ERROR = (1 << 2),
} Event;

CCMRAM static struct {
    uint8_t rx_buffer[2][UART_RX_BUFFER_SIZE];
    uint16_t rx_count[2];
    uint8_t rx_buffer_index;

    void* event;

    struct {
        void (*onDataReceived)(const uint8_t* data, uint16_t size);
    } callback;

    AppErrorInfo error_info;
} ui_comm_uart_private;

static void _onDataTransmitComplete(UART_HandleTypeDef* huart) {
    (void)huart;
    (void)osEventFlagsSet(self.event, EVENT_UART_TX_COMPLETE);
}

static void _onDataReceived(UART_HandleTypeDef* huart, uint16_t size) {
    (void)huart;
    self.rx_count[self.rx_buffer_index] = size;
    (void)osEventFlagsSet(self.event, EVENT_UART_RX_COMPLETE);
}

static void _onError(UART_HandleTypeDef* huart) {
    (void)huart;
    (void)osEventFlagsSet(self.event, EVENT_UART_ERROR);
}

static AppResult _resetUart() {
    if (HAL_UART_DeInit(UART_UI_COMM) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_UART_HAL_DEINIT);
        return APP_RESULT_FAIL;
    }

    if (HAL_UART_Init(UART_UI_COMM) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_UART_HAL_INIT);
        return APP_RESULT_FAIL;
    }

    (void)HAL_UART_RegisterCallback(UART_UI_COMM, HAL_UART_TX_COMPLETE_CB_ID,
                                    _onDataTransmitComplete);
    (void)HAL_UART_RegisterRxEventCallback(UART_UI_COMM, _onDataReceived);
    (void)HAL_UART_RegisterCallback(UART_UI_COMM, HAL_UART_ERROR_CB_ID, _onError);

    self.rx_buffer_index = 0;
    memset(self.rx_count, 0, sizeof(self.rx_count));

    return APP_RESULT_OK;
}

static uint32_t _calculateTxTimeout(uint16_t size) {
    uint32_t baudrate = UART_UI_COMM->Init.BaudRate;
    if (baudrate == 0) {
        return UART_TX_TIMEOUT_MARGIN_MS;
    }

    uint32_t transmit_time_ms = ((uint32_t)size * UART_TX_BITS_PER_BYTE * 1000u) / baudrate;
    return transmit_time_ms + UART_TX_TIMEOUT_MARGIN_MS;
}

AppResult UiCommUart_Initialize(const UiCommCallback* callback) {
    if (callback == NULL || callback->onDataReceived == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    self.callback.onDataReceived = callback->onDataReceived;

    if (self.event == NULL) {
        self.event = osEventFlagsNew(NULL);
        if (self.event == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_EVENT_CREATE);
            return APP_RESULT_FAIL;
        }
    }

    if (_resetUart() != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    if (HAL_UARTEx_ReceiveToIdle_IT(UART_UI_COMM, self.rx_buffer[self.rx_buffer_index],
                                    UART_RX_BUFFER_SIZE) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_UART_HAL_READ);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult UiCommUart_Deinitialize(void) { return APP_RESULT_OK; }

void UiCommUart_GetLastErrorInfo(AppErrorInfo* error_info) {
    if (error_info != NULL) {
        *error_info = self.error_info;
    }
}

void UiCommUart_Process(uint32_t timeout_tick) {
    uint32_t flag = osEventFlagsWait(self.event, EVENT_UART_RX_COMPLETE | EVENT_UART_ERROR,
                                     osFlagsWaitAny, timeout_tick);
    if (flag == osFlagsErrorTimeout) {
        return;
    } else if (flag & osFlagsError) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_EVENT_WAIT);
        return;
    }

    if (flag & EVENT_UART_RX_COMPLETE) {
        uint8_t* buffer = self.rx_buffer[self.rx_buffer_index];
        uint16_t count = self.rx_count[self.rx_buffer_index];

        self.callback.onDataReceived(buffer, count);

        self.rx_buffer_index = (self.rx_buffer_index + 1) % 2;

        if (HAL_UARTEx_ReceiveToIdle_IT(UART_UI_COMM, self.rx_buffer[self.rx_buffer_index],
                                        UART_RX_BUFFER_SIZE) != HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_UART_HAL_READ);
            return;
        }
    }

    if (flag & EVENT_UART_ERROR) {
        if (_resetUart() != APP_RESULT_OK) {
            return;
        }

        if (HAL_UARTEx_ReceiveToIdle_IT(UART_UI_COMM, self.rx_buffer[self.rx_buffer_index],
                                        UART_RX_BUFFER_SIZE) != HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_UART_HAL_READ);
            return;
        }
    }
}

AppResult UiCommUart_SendData(const uint8_t* data, uint16_t size) {
    if (data == NULL || size == 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    if (HAL_UART_Transmit(UART_UI_COMM, (uint8_t*)data, size, _calculateTxTimeout(size)) !=
        HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_UART_HAL_WRITE);
        return APP_RESULT_FAIL;
    }

    /*

    // Drop any stale flag left behind by a previous send that already timed out,
    // so it can't be mistaken for this transmit's completion below.
    (void)osEventFlagsClear(self.event, EVENT_UART_TX_COMPLETE | EVENT_UART_ERROR);

    if (HAL_UART_Transmit_IT(UART_UI_COMM, (uint8_t*)data, size) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_UART_HAL_WRITE);
        return APP_RESULT_FAIL;
    }

    uint32_t flag = EVENT_UART_TX_COMPLETE | EVENT_UART_ERROR;
    flag = osEventFlagsWait(self.event, flag, osFlagsWaitAny, _calculateTxTimeout(size));
    if (flag & osFlagsError || flag & EVENT_UART_ERROR) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_UART_HAL_WRITE_TIMEOUT);
        return APP_RESULT_FAIL;
    }

    */

    return APP_RESULT_OK;
}
