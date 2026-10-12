#include "../../domain/interface/external_comm.h"
#include "../../lib/comm_hgp/comm_hgp_can.h"
#include "../../lib/comm_hgp/comm_hgp_serial.h"
#include "../service/platform.h"

#define self (external_comm_private)

#define HGP_DEVICE_ADDRESS (0x60)

#define UART_RX_BUFFER_SIZE (64u)

#define PARSE_TIMEOUT_TICK (500u)

#define DATA_STREAM_TICK (500u)

#define ERROR_CHECK_TICK (1000u)
typedef enum {
    EVENT_UART_TX_COMPLETE = (1 << 0),
    EVENT_UART_RX_COMPLETE = (1 << 1),
    EVENT_UART_ERROR = (1 << 2),

    EVENT_CAN_TX_COMPLETE = (1 << 3),
    EVENT_CAN_RX_COMPLETE = (1 << 4),
    EVENT_CAN_ERROR = (1 << 5),

    EVENT_CAN_DATA_STREAM = (1 << 6),
} Event;

#pragma pack(push, 1)
typedef union {
    struct {
        CAN_RxHeaderTypeDef header;
        uint8_t data[8];
    };
    uint8_t raw[sizeof(CAN_RxHeaderTypeDef) + 8];
} CanRxMessage;

#pragma pack(pop)

static struct {
    struct {
        uint8_t rx_buffer[2][UART_RX_BUFFER_SIZE];
        uint16_t rx_count[2];
        uint8_t rx_buffer_index;
    } uart;

    struct {
        CanRxMessage rx_buffer[3];
        uint8_t rx_count;
    } can;

    void* event;
    void* serial_timeout_timer;
    void* can_stream_timer;
    void* error_check_timer;

    AppErrorInfo error_info;
    App* app;
} external_comm_private;

static void _onUartTxComplete(UART_HandleTypeDef* huart) {
    (void)huart;
    (void)osEventFlagsSet(self.event, EVENT_UART_TX_COMPLETE);
}

static void _onUartRxComplete(UART_HandleTypeDef* huart, uint16_t position) {
    (void)huart;
    self.uart.rx_count[self.uart.rx_buffer_index] = position;
    (void)osEventFlagsSet(self.event, EVENT_UART_RX_COMPLETE);
}

static void _onUartError(UART_HandleTypeDef* huart) {
    (void)huart;
    (void)osEventFlagsSet(self.event, EVENT_UART_ERROR);
}

static void _onCanTxComplete(CAN_HandleTypeDef* hcan) {
    (void)hcan;
    (void)osEventFlagsSet(self.event, EVENT_CAN_TX_COMPLETE);
}

static void _onCanRxComplete(CAN_HandleTypeDef* hcan) {
    uint32_t available = HAL_CAN_GetRxFifoFillLevel(CAN_HGP, CAN_RX_FIFO0);
    for (uint32_t i = 0; i < available; i++) {
        CanRxMessage* message = &self.can.rx_buffer[i];
        if (HAL_CAN_GetRxMessage(CAN_HGP, CAN_RX_FIFO0, &message->header, message->data) !=
            HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_READ);
        }
        self.can.rx_count++;
    }

    (void)osEventFlagsSet(self.event, EVENT_CAN_RX_COMPLETE);
}

static void _onCanError(CAN_HandleTypeDef* hcan) {
    (void)hcan;
    (void)osEventFlagsSet(self.event, EVENT_CAN_ERROR);
}

static void _onSerialTimeout(void* argument) {
    (void)argument;
    CommHgpSerial_ResetParseState();
}

static void _onCanStreamTimeout(void* argument) {
    (void)argument;
    (void)osEventFlagsSet(self.event, EVENT_CAN_DATA_STREAM);
}

static void _onErrorCheckTimeout(void* argument) {
    (void)argument;
    if (HAL_CAN_GetError(CAN_HGP) != HAL_CAN_ERROR_NONE) {
        (void)osEventFlagsSet(self.event, EVENT_CAN_ERROR);
    }

    if (HAL_UART_GetError(UART_HGP) != HAL_UART_ERROR_NONE) {
        (void)osEventFlagsSet(self.event, EVENT_UART_ERROR);
    }
}

static int _writeUart(const uint8_t* datas, uint16_t length) {
    if (HAL_UART_Transmit_DMA(UART_HGP, datas, length) != HAL_OK) {
        return -1;
    }

    uint32_t flag = EVENT_UART_TX_COMPLETE | EVENT_UART_ERROR;
    flag = osEventFlagsWait(self.event, flag, osFlagsWaitAny, 100);
    if (flag & osFlagsError || flag & EVENT_UART_ERROR) {
        return -1;
    }

    return 0;
}

static int _writeCan(uint32_t id, uint8_t ide, uint8_t rtr, uint8_t* data, uint16_t length) {
    if (length > 8) {
        return -1;
    }

    CAN_TxHeaderTypeDef tx_header = {
        .StdId = id,
        .IDE = ide > 0 ? CAN_ID_EXT : CAN_ID_STD,
        .RTR = rtr > 0 ? CAN_RTR_REMOTE : CAN_RTR_DATA,
        .DLC = length,
    };

    uint32_t tx_mailbox;
    if (HAL_CAN_AddTxMessage(CAN_HGP, &tx_header, data, &tx_mailbox) != HAL_OK) {
        return -1;
    }

    uint32_t flag = EVENT_CAN_TX_COMPLETE | EVENT_CAN_ERROR;
    flag = osEventFlagsWait(self.event, flag, osFlagsWaitAny, 100);
    if (flag & osFlagsError || flag & EVENT_CAN_ERROR) {
        return -1;
    }

    return 0;
}

static void _getStatus(CommHgpData* status) {
    BmsData data = self.app->data;

    uint32_t pack_voltage = 0;
    for (uint8_t i = 0; i < VOLTAGE_MAX_COUNT; i++) {
        pack_voltage += data.voltage.bank_voltages[i].voltage;
    }
    CommHgpStateData state = {
        .ovp = data.protection_state.ovp,
        .uvp = data.protection_state.uvp,
        .cocp = data.protection_state.cocp | data.protection_state.rocp,
        .docp = data.protection_state.docp,
        .otp = data.protection_state.otp | data.protection_state.cotp | data.protection_state.dotp |
               data.protection_state.rotp,
        .utp = data.protection_state.utp | data.protection_state.cutp | data.protection_state.dutp |
               data.protection_state.rutp,
    };

    status->pack_voltage = (pack_voltage + 5) / 10;  // Convert 0.001V to 0.01V, rounding
    status->pack_current = data.current.value * 10;  // Convert 0.1A to 0.01A, rounding
    status->soc = (data.soc.display_soc + 5) / 10;   // Convert 0.1% to %, rounding
    status->state = state;
    status->charge_remaining_time =
        self.app->data.expansion.charge_remaining_time / (1000 * 60);  // Convert ms to minutes
    status->discharge_remaining_time =
        self.app->data.expansion.discharge_remaining_time / (1000 * 60);  // Convert ms to minutes
    status->temperature = data.pack_temperature.max_temperature;
    status->soh = 100;  // TODO: calculate SOH
    status->remaining_capacity =
        (data.expansion.remain_capacity + 5) / 10;  // Convert mAh to 0.01Ah, rounding
    status->remaining_energy =
        (data.expansion.remain_energy + 50) / 100;  // Convert mWh to 0.1Wh, rounding
}

static void _ToHgpData(BmsData* bms_data, CommHgpData* hgp_data) {
    uint32_t pack_voltage = 0;
    for (uint8_t i = 0; i < VOLTAGE_MAX_COUNT; i++) {
        pack_voltage += bms_data->voltage.bank_voltages[i].voltage;
    }
    CommHgpStateData state = {
        .ovp = bms_data->protection_state.ovp,
        .uvp = bms_data->protection_state.uvp,
        .cocp = bms_data->protection_state.cocp | bms_data->protection_state.rocp,
        .docp = bms_data->protection_state.docp,
        .otp = bms_data->protection_state.otp | bms_data->protection_state.cotp |
               bms_data->protection_state.dotp | bms_data->protection_state.rotp,
        .utp = bms_data->protection_state.utp | bms_data->protection_state.cutp |
               bms_data->protection_state.dutp | bms_data->protection_state.rutp,
    };

    hgp_data->pack_voltage = (pack_voltage + 5) / 10;       // convert 0.001V to 0.01V, rounding
    hgp_data->pack_current = bms_data->current.value * 10;  // convert 0.1A to 0.01A, rounding
    hgp_data->soc = (bms_data->soc.display_soc + 5) / 10;   // convert 0.1% to %, rounding
    hgp_data->state = state;
    hgp_data->charge_remaining_time = (bms_data->expansion.charge_remaining_time + 30000) /
                                      60000;  // convert ms to minutes, rounding
    hgp_data->discharge_remaining_time = (bms_data->expansion.discharge_remaining_time + 30000) /
                                         60000;  // convert ms to minutes, rounding
    hgp_data->temperature = bms_data->pack_temperature.max_temperature;
    hgp_data->soh = 100;  // TODO: calculate SOH
    hgp_data->remaining_capacity =
        (bms_data->expansion.remain_capacity + 5) / 10;  // convert mAh to 0.01Ah, rounding
    hgp_data->remaining_energy =
        (bms_data->expansion.remain_energy + 50) / 100;  // convert mWh to 0.1Wh, rounding
}

static void _onParseStarted() { (void)osTimerStart(self.serial_timeout_timer, PARSE_TIMEOUT_TICK); }

static void _onParseCompleted() { (void)osTimerStop(self.serial_timeout_timer); }

static void _onCommandReceived(uint8_t address, uint8_t command) {
    (void)address;
    (void)command;
}

static void _onErrorOccurred(uint8_t error_code) { (void)error_code; }

static void _onCanDataRequested() { (void)osEventFlagsSet(self.event, EVENT_CAN_DATA_STREAM); }

static void _onCanDataStreamStartRequested() {
    (void)osTimerStart(self.can_stream_timer, DATA_STREAM_TICK);
}

static void _onCanDataStreamStopRequested() { (void)osTimerStop(self.can_stream_timer); }

static AppResult _resetUart(void) {
    if (HAL_UART_DeInit(UART_HGP) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_DEINIT);
        return APP_RESULT_FAIL;
    }

    if (HAL_UART_Init(UART_HGP) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_INIT);
        return APP_RESULT_FAIL;
    }

    (void)HAL_UART_RegisterCallback(UART_HGP, HAL_UART_TX_COMPLETE_CB_ID, _onUartTxComplete);
    (void)HAL_UART_RegisterRxEventCallback(UART_HGP, _onUartRxComplete);
    (void)HAL_UART_RegisterCallback(UART_HGP, HAL_UART_ERROR_CB_ID, _onUartError);

    self.uart.rx_buffer_index = 0;
    memset(self.uart.rx_count, 0, sizeof(self.uart.rx_count));

    return APP_RESULT_OK;
}

static AppResult _resetCan(void) {
    if (HAL_CAN_DeInit(CAN_HGP) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_DEINIT);
        return APP_RESULT_FAIL;
    }

    if (HAL_CAN_Init(CAN_HGP) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_INIT);
        return APP_RESULT_FAIL;
    }

    (void)HAL_CAN_RegisterCallback(CAN_HGP, HAL_CAN_TX_MAILBOX0_COMPLETE_CB_ID, _onCanTxComplete);
    (void)HAL_CAN_RegisterCallback(CAN_HGP, HAL_CAN_TX_MAILBOX1_COMPLETE_CB_ID, _onCanTxComplete);
    (void)HAL_CAN_RegisterCallback(CAN_HGP, HAL_CAN_TX_MAILBOX2_COMPLETE_CB_ID, _onCanTxComplete);
    (void)HAL_CAN_RegisterCallback(CAN_HGP, HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID, _onCanRxComplete);
    (void)HAL_CAN_RegisterCallback(CAN_HGP, HAL_CAN_RX_FIFO0_FULL_CB_ID, _onCanRxComplete);
    (void)HAL_CAN_RegisterCallback(CAN_HGP, HAL_CAN_ERROR_CB_ID, _onCanError);

    uint32_t notifications = CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING |
                             CAN_IT_RX_FIFO0_FULL | CAN_IT_RX_FIFO0_OVERRUN | CAN_IT_ERROR_WARNING |
                             CAN_IT_ERROR_PASSIVE | CAN_IT_BUSOFF;

    if (HAL_CAN_ActivateNotification(CAN_HGP, notifications) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_INIT);
        return APP_RESULT_FAIL;
    }

    uint32_t filter_id = HGP_DEVICE_ADDRESS | HGP_CAN_BASE_ID;

    CAN_FilterTypeDef filter_config = {
        .FilterBank = 14,  // CAN2 slave banks start at 14 (default SlaveStartFilterBank)
        .FilterMode = CAN_FILTERMODE_IDMASK,
        .FilterScale = CAN_FILTERSCALE_32BIT,

        .FilterIdHigh = (filter_id << 5) & 0xFFFF,
        .FilterIdLow = 0x0000,
        .FilterMaskIdHigh = 0xFFFF,
        .FilterMaskIdLow = 0xFFFF,

        // .FilterIdHigh = 0x0000,
        // .FilterIdLow = 0x0000,
        // .FilterMaskIdHigh = 0x0000,
        // .FilterMaskIdLow = 0x0000,

        .FilterFIFOAssignment = CAN_FILTER_FIFO0,
        .FilterActivation = ENABLE,
    };

    if (HAL_CAN_ConfigFilter(CAN_HGP, &filter_config) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_INIT);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult ExternalComm_Initialize(App* app) {
    if (app == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    self.app = app;

    if (self.event == NULL) {
        osEventFlagsId_t event = osEventFlagsNew(NULL);
        if (event == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_EVENT_CREATE);
            return APP_RESULT_FAIL;
        }
        self.event = event;
    }

    if (self.serial_timeout_timer == NULL) {
        osTimerId_t serial_timeout_timer = osTimerNew(_onSerialTimeout, osTimerOnce, NULL, NULL);
        if (serial_timeout_timer == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_TIMER_CREATE);
            return APP_RESULT_FAIL;
        }
        self.serial_timeout_timer = serial_timeout_timer;
        osDelay(5);
    }

    if (self.can_stream_timer == NULL) {
        osTimerId_t can_stream_timer = osTimerNew(_onCanStreamTimeout, osTimerPeriodic, NULL, NULL);
        if (can_stream_timer == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_TIMER_CREATE);
            return APP_RESULT_FAIL;
        }
        self.can_stream_timer = can_stream_timer;
        osDelay(5);
    }

    if (self.error_check_timer == NULL) {
        osTimerId_t error_check_timer =
            osTimerNew(_onErrorCheckTimeout, osTimerPeriodic, NULL, NULL);
        if (error_check_timer == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_TIMER_CREATE);
            return APP_RESULT_FAIL;
        }
        self.error_check_timer = error_check_timer;
        osDelay(5);
    }

    if (CommHgpSerial_Initialize(
            &(CommHgpConfig){
                .device_address = HGP_DEVICE_ADDRESS,
            },
            &(CommHgpSerialInterface){
                .write = _writeUart,
                .getStatus = _getStatus,
            },
            &(CommHgpSerialCallback){
                .onParseCompleted = _onParseCompleted,
                .onParseStarted = _onParseStarted,
                .onCommandReceived = _onCommandReceived,
                .onErrorOccurred = _onErrorOccurred,
            }) != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_DRV_INIT);
        return APP_RESULT_FAIL;
    }

    if (CommHgpCan_Initialize(
            &(CommHgpConfig){
                .device_address = HGP_DEVICE_ADDRESS,
            },
            &(CommHgpCanInterface){
                .write = _writeCan,
            },
            &(CommHgpCanCallback){
                .onDataRequested = _onCanDataRequested,
                .onDataStreamStartRequested = _onCanDataStreamStartRequested,
                .onDataStreamStopRequested = _onCanDataStreamStopRequested,
            }) != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_DRV_INIT);
        return APP_RESULT_FAIL;
    }

    if (_resetUart() != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    if (_resetCan() != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    if (HAL_UARTEx_ReceiveToIdle_DMA(UART_HGP, self.uart.rx_buffer[self.uart.rx_buffer_index],
                                     UART_RX_BUFFER_SIZE) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_READ);
        return APP_RESULT_FAIL;
    }

    if (HAL_CAN_Start(CAN_HGP) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_READ);
        return APP_RESULT_FAIL;
    }

    if (osTimerIsRunning(self.error_check_timer) == 0) {
        if (osTimerStart(self.error_check_timer, ERROR_CHECK_TICK) != osOK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_TIMER_START);
            return APP_RESULT_FAIL;
        }
    }

    return APP_RESULT_OK;
}

AppResult ExternalComm_Deinitialize(void) { return APP_RESULT_OK; }

AppResult ExternalComm_Process() {
    uint32_t flag =
        osEventFlagsWait(self.event,
                         EVENT_UART_RX_COMPLETE | EVENT_UART_ERROR | EVENT_CAN_RX_COMPLETE |
                             EVENT_CAN_ERROR | EVENT_CAN_DATA_STREAM,
                         osFlagsWaitAny, osWaitForever);
    if (flag == osFlagsErrorTimeout) {
        return APP_RESULT_OK;
    } else if (flag & osFlagsError) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_EVENT_WAIT);
        return APP_RESULT_FAIL;
    }

    if (flag & EVENT_UART_RX_COMPLETE) {
        uint8_t* buffer = self.uart.rx_buffer[self.uart.rx_buffer_index];
        uint16_t count = self.uart.rx_count[self.uart.rx_buffer_index];

        if (CommHgpSerial_Parse(buffer, count) != 0) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_WRITE);
            return APP_RESULT_FAIL;
        }

        self.uart.rx_buffer_index = (self.uart.rx_buffer_index + 1) % 2;

        if (HAL_UARTEx_ReceiveToIdle_DMA(UART_HGP, self.uart.rx_buffer[self.uart.rx_buffer_index],
                                         UART_RX_BUFFER_SIZE) != HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_READ);
            return APP_RESULT_FAIL;
        }
    }

    if (flag & EVENT_UART_ERROR) {
        if (_resetUart() != APP_RESULT_OK) {
            return APP_RESULT_FAIL;
        }

        if (HAL_UARTEx_ReceiveToIdle_DMA(UART_HGP, self.uart.rx_buffer[self.uart.rx_buffer_index],
                                         UART_RX_BUFFER_SIZE) != HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_READ);
            return APP_RESULT_FAIL;
        }
    }

    if (flag & EVENT_CAN_RX_COMPLETE) {
        uint8_t available = self.can.rx_count;

        self.can.rx_count = 0;

        for (uint8_t i = 0; i < available; i++) {
            CanRxMessage* message = &self.can.rx_buffer[i];
            (void)CommHgpCan_Parse(message->header.StdId, message->data, message->header.DLC);
        }
    }

    if (flag & EVENT_CAN_DATA_STREAM) {
        BmsData bms_data = self.app->data;
        CommHgpData data = {0};

        _ToHgpData(&bms_data, &data);

        if (CommHgpCan_SendData(&data) != COMM_HGP_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_WRITE);
            return APP_RESULT_FAIL;
        }
    }

    if (flag & EVENT_CAN_ERROR) {
        if (_resetCan() != APP_RESULT_OK) {
            return APP_RESULT_FAIL;
        }

        if (HAL_CAN_Start(CAN_HGP) != HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EXTERNAL_COMM_HAL_READ);
            return APP_RESULT_FAIL;
        }
    }

    return APP_RESULT_OK;
}
