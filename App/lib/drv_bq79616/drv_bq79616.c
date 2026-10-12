//=============================================================================================================================

#include "drv_bq79616.h"

#include "drv_bq79616_register_map.h"

// Global variables
//=============================================================================================================================

const DrvBq79616 drv_bq79616;

// Private macro definitions
//=============================================================================================================================

#define PRIVATE drv_bq79616_private

#define MAX_DEVICE_ADDRESS (DRV_BQ79616_MAX_DEVICE_ADDRESS)
#define BROADCAST_DEVICE_ADDRESS (MAX_DEVICE_ADDRESS)
#define MAX_CELL_COUNT (DRV_BQ79616_MAX_CELL_COUNT)
#define MAX_GPIO_COUNT (DRV_BQ79616_MAX_GPIO_COUNT)
#define MIN_WRITE_DATA_SIZE (1u)
#define MAX_WRITE_DATA_SIZE (8u)
#define MIN_READ_DATA_SIZE (1u)
#define MAX_READ_DATA_SIZE (128u)
#define MAX_TIMEOUT (5000u)
#define CRC16_INIT_VALUE (0xFFFF)

#define PART_ID_BQ79616 (0x21)
#define PART_ID_BQ79614 (0x01)
#define PART_ID_BQ79612 (0x02)
#define PART_ID_BQ75614 (0x03)
#define PART_ID_BQ79656 (0x04)
#define PART_ID_BQ79654 (0x05)
#define PART_ID_BQ79652 (0x06)
#define PART_ID_BQ756506 (0x0A)

// Private Type definitions
//=============================================================================================================================

typedef enum {
    REQUEST_SINGLE_DEVICE_READ,
    REQUEST_SINGLE_DEVICE_WRITE,
    REQUEST_STACK_DEVICE_READ,
    REQUEST_STACK_DEVICE_WRITE,
    REQUEST_BROADCAST_DEVICE_READ,
    REQUEST_BROADCAST_DEVICE_WRITE,
    REQUEST_BROADCAST_DEVICE_WRITE_REVERSE
} RequestType;

#pragma pack(push, 1)

typedef struct {
    uint8_t data_size : 3;
    uint8_t rsvd : 1;
    uint8_t req_type : 3;
    uint8_t frame_type : 1;
} CommandInitializeByte;

typedef struct {
    uint8_t response_byte : 7;
    uint8_t frame_type : 1;
} ResponseInitializeByte;

typedef union {
    CommandInitializeByte command;
    ResponseInitializeByte response;
} InitializeByte;

typedef struct DrvBq79616Private {
    bool is_initialized;
    uint8_t daisy_chain_count;

    uint32_t frame_error_times;
    uint32_t crc_error_times;
    DrvBq79616Interface interface;
} DrvBq79616Private;

#pragma pack(pop)

// Private variables
//=============================================================================================================================

static DrvBq79616Private drv_bq79616_private;

static uint16_t crc16_table[256] = {
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241, 0xc601, 0x06c0, 0x0780, 0xc741,
    0x0500, 0xc5c1, 0xc481, 0x0440, 0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
    0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841, 0xd801, 0x18c0, 0x1980, 0xd941,
    0x1b00, 0xdbc1, 0xda81, 0x1a40, 0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
    0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641, 0xd201, 0x12c0, 0x1380, 0xd341,
    0x1100, 0xd1c1, 0xd081, 0x1040, 0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
    0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441, 0x3c00, 0xfcc1, 0xfd81, 0x3d40,
    0xff01, 0x3fc0, 0x3e80, 0xfe41, 0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
    0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41, 0xee01, 0x2ec0, 0x2f80, 0xef41,
    0x2d00, 0xedc1, 0xec81, 0x2c40, 0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
    0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041, 0xa001, 0x60c0, 0x6180, 0xa141,
    0x6300, 0xa3c1, 0xa281, 0x6240, 0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
    0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41, 0xaa01, 0x6ac0, 0x6b80, 0xab41,
    0x6900, 0xa9c1, 0xa881, 0x6840, 0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
    0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40, 0xb401, 0x74c0, 0x7580, 0xb541,
    0x7700, 0xb7c1, 0xb681, 0x7640, 0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
    0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241, 0x9601, 0x56c0, 0x5780, 0x9741,
    0x5500, 0x95c1, 0x9481, 0x5440, 0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
    0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841, 0x8801, 0x48c0, 0x4980, 0x8941,
    0x4b00, 0x8bc1, 0x8a81, 0x4a40, 0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
    0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641, 0x8201, 0x42c0, 0x4380, 0x8341,
    0x4100, 0x81c1, 0x8081, 0x4040};

// Private function prototypes
//=============================================================================================================================

static void ping(DrvBq79616Ping ping_type);

// Private function definitions
//=============================================================================================================================

static inline bool isValidDeviceAddress(uint8_t address) { return address <= MAX_DEVICE_ADDRESS; }

static int16_t toDecimal(uint16_t value) {
    int16_t result = 0;

    if (value & 0x8000) {
        result = -(int16_t)(~value + 1);
    } else {
        result = (int16_t)value;
    }

    return result;
}

static inline double toCellVoltage(uint16_t value) {
    return ((double)toDecimal(value) * 190.73) / (1000.0 * 1000.0);
}

static inline double toGpioVoltage(uint16_t value) {
    return ((double)toDecimal(value) * 152.59) / (1000.0 * 1000.0);
}

static inline double toTsRefVoltage(uint16_t value) {
    return ((double)toDecimal(value) * 169.54) / (1000.0 * 1000.0);
}

static inline double toTemperature(uint16_t value) { return (double)toDecimal(value) * 0.025; }

static uint16_t getCrc16(uint16_t init_value, uint8_t* datas, uint8_t length) {
    uint16_t crc = init_value;
    const uint8_t* p = datas;

    while (length--) {
        crc = crc16_table[(crc ^ (*p++)) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

static inline uint16_t getCommandFrameSize(const uint8_t data_size, const bool is_broadcast) {
    return is_broadcast ? data_size + sizeof(InitializeByte) +  // initialize byte
                              2 +                               // register address
                              2                                 // crc16
                        : data_size + sizeof(InitializeByte) +  // initialize byte
                              1 +                               // device address
                              2 +                               // register address
                              2;                                // crc16
}

static inline uint16_t getResponseFrameSize(const uint8_t data_size,
                                            const uint8_t daisy_chain_count) {
    return getCommandFrameSize(data_size, false) * daisy_chain_count;
}

static DrvBq79616Result deviceWrite(uint8_t device_address, uint16_t register_address,
                                    uint8_t* data, uint8_t size) {
    const bool is_broadcast = device_address == BROADCAST_DEVICE_ADDRESS;
    const uint16_t packet_size = getCommandFrameSize(size, is_broadcast);

    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;
    uint8_t buffer[packet_size];
    uint16_t count = 0;
    uint16_t crc = CRC16_INIT_VALUE;

    ((InitializeByte*)&buffer[0])->command.data_size = size - 1;
    ((InitializeByte*)&buffer[0])->command.rsvd = 0;
    ((InitializeByte*)&buffer[0])->command.frame_type = 1;

    if (is_broadcast) {
        ((InitializeByte*)&buffer[0])->command.req_type = REQUEST_BROADCAST_DEVICE_WRITE;
        buffer[1] = register_address >> 8;
        buffer[2] = register_address & 0xFF;

        memcpy(&buffer[3], data, size);
    } else {
        ((InitializeByte*)&buffer[0])->command.req_type = REQUEST_SINGLE_DEVICE_WRITE;
        buffer[1] = device_address;
        buffer[2] = register_address >> 8;
        buffer[3] = register_address & 0xFF;

        memcpy(&buffer[4], data, size);
    }

    crc = getCrc16(crc, buffer, packet_size - sizeof(crc));
    *(uint16_t*)&buffer[packet_size - sizeof(crc)] = crc;

    result = PRIVATE.interface.writeUart(buffer, packet_size);
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    while (!PRIVATE.interface.isUartTxReady()) {
        PRIVATE.interface.delayCycle();
        if (++count >= MAX_TIMEOUT) {
            return DRV_BQ79616_RESULT_TIMEOUT;
        }
    }

    return result;
}

static DrvBq79616Result deviceRead(uint8_t device_address, uint16_t register_address, uint8_t* data,
                                   uint8_t size) {
    const bool is_broadcast = device_address == BROADCAST_DEVICE_ADDRESS;
    const uint16_t tx_packet_size = getCommandFrameSize(1, is_broadcast);
    const uint16_t rx_packet_size = getResponseFrameSize(size, PRIVATE.daisy_chain_count);

    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;
    uint16_t count = 0;
    uint16_t tx_crc = CRC16_INIT_VALUE;
    uint16_t rx_crc = CRC16_INIT_VALUE;
    uint8_t tx_buffer[tx_packet_size];
    uint8_t rx_buffer[rx_packet_size];

    ((InitializeByte*)&tx_buffer[0])->command.data_size = 0;
    ((InitializeByte*)&tx_buffer[0])->command.rsvd = 0;
    ((InitializeByte*)&tx_buffer[0])->command.frame_type = 1;

    if (is_broadcast) {
        ((InitializeByte*)&tx_buffer[0])->command.req_type = REQUEST_BROADCAST_DEVICE_READ;
        tx_buffer[1] = register_address >> 8;
        tx_buffer[2] = register_address & 0xFF;
        tx_buffer[3] = size - 1;
    } else {
        ((InitializeByte*)&tx_buffer[0])->command.req_type = REQUEST_SINGLE_DEVICE_READ;
        tx_buffer[1] = device_address;
        tx_buffer[2] = register_address >> 8;
        tx_buffer[3] = register_address & 0xFF;
        tx_buffer[4] = size - 1;
    }

    tx_crc = getCrc16(tx_crc, tx_buffer, tx_packet_size - sizeof(tx_crc));
    *(uint16_t*)&tx_buffer[tx_packet_size - sizeof(tx_crc)] = tx_crc;

    result = PRIVATE.interface.listenUart(rx_buffer, rx_packet_size);
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    result = PRIVATE.interface.writeUart(tx_buffer, tx_packet_size);
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    while (!PRIVATE.interface.isUartTxReady()) {
        PRIVATE.interface.delayCycle();
        if (++count >= MAX_TIMEOUT) {
            return DRV_BQ79616_RESULT_TIMEOUT;
        }
    }

    count = 0;

    while (!PRIVATE.interface.isUartRxReady()) {
        PRIVATE.interface.delayCycle();
        if (++count >= MAX_TIMEOUT) {
            return DRV_BQ79616_RESULT_TIMEOUT;
        }
    }

    if (is_broadcast) {
        const uint16_t packet_size = rx_packet_size / PRIVATE.daisy_chain_count;

        for (uint8_t i = 0; i < PRIVATE.daisy_chain_count; ++i) {
            rx_crc = CRC16_INIT_VALUE;

            if (((InitializeByte*)&rx_buffer[i * packet_size])->response.frame_type == 1) {
                ++PRIVATE.frame_error_times;
                return DRV_BQ79616_RESULT_FAIL;
            }

            rx_crc = getCrc16(rx_crc, &rx_buffer[i * packet_size], packet_size - sizeof(rx_crc));
            if (*(uint16_t*)&rx_buffer[((i + 1) * packet_size) - sizeof(rx_crc)] != rx_crc) {
                ++PRIVATE.crc_error_times;
                return DRV_BQ79616_RESULT_FAIL;
            }

            memcpy(data + i, &rx_buffer[4 + (i * packet_size)],
                   ((InitializeByte*)&rx_buffer[(i * packet_size)])->response.response_byte + 1);
        }
    } else {
        if (((InitializeByte*)&rx_buffer[0])->response.frame_type == 1) {
            ++PRIVATE.frame_error_times;
            return DRV_BQ79616_RESULT_FAIL;
        }

        rx_crc = getCrc16(rx_crc, rx_buffer, rx_packet_size - sizeof(rx_crc));
        if (*(uint16_t*)&rx_buffer[rx_packet_size - sizeof(rx_crc)] != rx_crc) {
            ++PRIVATE.crc_error_times;
            return DRV_BQ79616_RESULT_FAIL;
        }

        memcpy(data, &rx_buffer[4], ((InitializeByte*)&rx_buffer[0])->response.response_byte + 1);
    }

    return result;
}

static DrvBq79616Result syncWriteDelayLockedLoop() {
    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;
    uint8_t reg = 0;

    (void)deviceWrite(BROADCAST_DEVICE_ADDRESS, OTP_ECC_TEST, &reg, sizeof(reg));

    PRIVATE.interface.delayMs(3);

    return result;
}

static DrvBq79616Result syncReadDelayLockedLoop() {
    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;
    uint8_t reg = 0;

    result = deviceRead(BROADCAST_DEVICE_ADDRESS, OTP_ECC_TEST, &reg, sizeof(reg));
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    return result;
}

static DrvBq79616Result startAutoAddressing() {
    Control1 reg = {.addr_wr = 1};

    return deviceWrite(BROADCAST_DEVICE_ADDRESS, CONTROL1, (uint8_t*)&reg, sizeof(reg));
}

static DrvBq79616Result setDeviceAddress(uint8_t device_address, bool direction) {
    DirAddr reg = {.address = device_address};

    return deviceWrite(BROADCAST_DEVICE_ADDRESS, direction ? DIR1_ADDR : DIR0_ADDR, (uint8_t*)&reg,
                       sizeof(reg));
}

static DrvBq79616Result setCommCtrl(uint8_t device_address, bool is_top, bool is_stack) {
    CommCtrl reg = {.top_stack = is_top, .stack_dev = is_stack};

    return deviceWrite(device_address, COMM_CTRL, (uint8_t*)&reg, sizeof(reg));
}

static DrvBq79616Result checkId(uint8_t device_address) {
    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;
    uint8_t reg = 0;

    result = deviceRead(device_address, PARTID, &reg, sizeof(reg));
    if ((result != DRV_BQ79616_RESULT_OK) || (reg != PART_ID_BQ79616)) {
        return DRV_BQ79616_RESULT_FAIL;
    }

    result = deviceRead(device_address, DEV_REVID, &reg, sizeof(reg));
    if ((result != DRV_BQ79616_RESULT_OK) || (reg != 0)) {
        return DRV_BQ79616_RESULT_FAIL;
    }

    return result;
}

/*
static DrvBq79616Result setHoldOffTime(uint8_t device_address, uint32_t tick)
{
    uint8_t reg = tick;

    return deviceWrite(
        device_address,
        COMM_CTRL,
        &reg,
        sizeof(reg));
}
*/

// Public method definitions
//=============================================================================================================================

static DrvBq79616Result initialize(DrvBq79616Interface* interface, uint8_t daisy_chain_count) {
    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;

    for (uint8_t i = 0; i < (sizeof(DrvBq79616Interface) / sizeof(void*)); ++i) {
        if ((void*)interface + i == NULL) {
            return DRV_BQ79616_RESULT_INVALID_ARG;
        }
    }

    if (daisy_chain_count < 1) {
        return DRV_BQ79616_RESULT_INVALID_ARG;
    }

    memcpy(&PRIVATE.interface, interface, sizeof(DrvBq79616Interface));

    PRIVATE.daisy_chain_count = daisy_chain_count;

    ping(DRV_BQ79616_PING_WAKE);  // waiting for minimum 2.7ms to the device woke up

    PRIVATE.interface.delayMs(50);

    result = syncWriteDelayLockedLoop();
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    result = startAutoAddressing();
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    for (uint8_t address = 0; address < PRIVATE.daisy_chain_count; ++address) {
        result = setDeviceAddress(address, 0);
        if (result != DRV_BQ79616_RESULT_OK) {
            return result;
        }
    }

    result = setCommCtrl(BROADCAST_DEVICE_ADDRESS, false, true);
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    if (daisy_chain_count > 1) {
        result = setCommCtrl(0, false, true);
        if (result != DRV_BQ79616_RESULT_OK) {
            return result;
        }

        result = setCommCtrl(daisy_chain_count, true, false);
        if (result != DRV_BQ79616_RESULT_OK) {
            return result;
        }
    } else {
        result = setCommCtrl(0, true, false);
        if (result != DRV_BQ79616_RESULT_OK) {
            return result;
        }
    }

    result = syncReadDelayLockedLoop();
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    result = checkId(BROADCAST_DEVICE_ADDRESS);
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    return result;
}

static DrvBq79616Result deinitialize() { return DRV_BQ79616_RESULT_OK; }

static void ping(DrvBq79616Ping type) {
    PRIVATE.interface.switchUartToGpio(1, 1);
    PRIVATE.interface.writeTxPin(0);

    switch (type) {
        case DRV_BQ79616_PING_SLEEP_TO_ACTIVE: {
            PRIVATE.interface.delayUs(275);
            break;
        }
        case DRV_BQ79616_PING_WAKE: {
            PRIVATE.interface.delayMs(3);
            break;
        }
        case DRV_BQ79616_PING_SHUTDOWN: {
            PRIVATE.interface.delayMs(8);
            break;
        }
        case DRV_BQ79616_PING_HW_RESET: {
            PRIVATE.interface.delayMs(40);
            break;
        }
    }

    PRIVATE.interface.writeTxPin(1);
    PRIVATE.interface.switchGpioToUart();
}

static DrvBq79616Result setConfiguration(uint8_t device_address, DrvBq79616Config* config) {
    if (!isValidDeviceAddress(device_address) || (config == NULL)) {
        return DRV_BQ79616_RESULT_INVALID_ARG;
    }

    if (config->active_cell_count < 6) {
        return DRV_BQ79616_RESULT_INVALID_ARG;
    }

    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;
    DevConf dev_conf = {// default values
                        .rsvd = 0,        .no_adj_cb = 1, .multidrop_en = 0, .fcomm_en = 1,
                        .two_stop_en = 0, .nfault_en = 1, .ftone_en = 0,     .hb_en = 0};
    ActiveCell active_cell = {.num_cell = config->active_cell_count - 6};
    AdcConf1 adc_conf1 = {.lpf_vcell = config->lpf_option};
    UvThresh uv_thresh = {0};
    OvThresh ov_thresh = {0};
    OvUvCtrl ovuv_ctrl = {.ovuv_go = config->is_voltage_protector_enabled ? 1 : 0,
                          .ovuv_mode = config->is_voltage_protector_enabled ? 1 : 0};
    OtUtThresh otut_thresh = {// default values
                              .ot_thresh = 0b111,
                              .ut_thresh = 0};
    OtUtCtrl otut_ctrl = {.otut_go = config->is_temperature_protector_enabled ? 1 : 0,
                          .otut_mode = config->is_temperature_protector_enabled ? 1 : 0};
    GpioConf gpio_conf[4] = {0};

    for (uint8_t i = 0; i < MAX_GPIO_COUNT; i += 2) {
        gpio_conf[i / 2].gpio_0 = config->gpio_roles[i] & 0x07;
        gpio_conf[i / 2].gpio_1 = config->gpio_roles[i + 1] & 0x07;
    }

    if (config->under_voltage_protector_threshold < 1.2) {
        uv_thresh.uv_thr = 0x00;
    } else if ((config->under_voltage_protector_threshold >= 1.2) &&
               (config->under_voltage_protector_threshold <= 3.1)) {
        uv_thresh.uv_thr = (uint32_t)((config->under_voltage_protector_threshold - 1.2) / 0.050);
    } else {
        uv_thresh.uv_thr = 0x26;
    }

    if ((config->over_voltage_protector_threshold < 2.7)) {
        ov_thresh.ov_thr = 0x02;
    } else if ((config->over_voltage_protector_threshold >= 2.7) &&
               (config->over_voltage_protector_threshold <= 3.0)) {
        ov_thresh.ov_thr =
            0x02 + (uint32_t)((config->over_voltage_protector_threshold - 2.7) / 0.025);
    } else if ((config->over_voltage_protector_threshold > 3.0) &&
               (config->over_voltage_protector_threshold < 3.5)) {
        ov_thresh.ov_thr = 0x0E;
    } else if ((config->over_voltage_protector_threshold >= 3.5) &&
               (config->over_voltage_protector_threshold <= 3.8)) {
        ov_thresh.ov_thr =
            0x12 + (uint32_t)((config->over_voltage_protector_threshold - 3.5) / 0.025);
    } else if ((config->over_voltage_protector_threshold > 3.8) &&
               (config->over_voltage_protector_threshold < 4.175)) {
        ov_thresh.ov_thr = 0x22;
    } else if ((config->over_voltage_protector_threshold >= 4.175) &&
               (config->over_voltage_protector_threshold <= 4.475)) {
        ov_thresh.ov_thr =
            0x22 + (uint32_t)((config->over_voltage_protector_threshold - 4.175) / 0.025);
    } else {
        ov_thresh.ov_thr = 0x3F;
    }

    uint8_t* regs[] = {(uint8_t*)&dev_conf,     (uint8_t*)&active_cell,  (uint8_t*)&adc_conf1,
                       (uint8_t*)&uv_thresh,    (uint8_t*)&ov_thresh,    (uint8_t*)&ovuv_ctrl,
                       (uint8_t*)&otut_thresh,  (uint8_t*)&otut_ctrl,    (uint8_t*)&gpio_conf[0],
                       (uint8_t*)&gpio_conf[1], (uint8_t*)&gpio_conf[2], (uint8_t*)&gpio_conf[3]};
    uint16_t reg_addresses[] = {DEV_CONF,   ACTIVE_CELL, ADC_CONF1,   UV_THRESH,
                                OV_THRESH,  OVUV_CTRL,   OTUT_THRESH, OTUT_CTRL,
                                GPIO_CONF1, GPIO_CONF2,  GPIO_CONF3,  GPIO_CONF4};

    for (uint8_t i = 0; i < sizeof(regs) / sizeof(regs[0]); ++i) {
        result = deviceWrite(device_address, reg_addresses[i], regs[i], sizeof(uint8_t));
        if (result != DRV_BQ79616_RESULT_OK) {
            return DRV_BQ79616_RESULT_FAIL;
        }
    }

    return result;
}

static DrvBq79616Result getConfiguration(uint8_t device_address, DrvBq79616Config* config) {
    if (!isValidDeviceAddress(device_address) || (config == NULL)) {
        return DRV_BQ79616_RESULT_INVALID_ARG;
    }

    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;

    DevConf dev_conf = {0};
    ActiveCell active_cell = {0};
    AdcConf1 adc_conf1 = {0};

    uint8_t* regs[] = {(uint8_t*)&dev_conf, (uint8_t*)&active_cell, (uint8_t*)&adc_conf1};
    uint16_t reg_addresses[] = {DEV_CONF, ACTIVE_CELL, ADC_CONF1};

    for (uint8_t i = 0; i < sizeof(regs) / sizeof(regs[0]); ++i) {
        result = deviceRead(device_address, reg_addresses[i], regs[i], sizeof(uint8_t));
        if (result != DRV_BQ79616_RESULT_OK) {
            return DRV_BQ79616_RESULT_FAIL;
        }
    }

    config->active_cell_count = active_cell.num_cell + 6;

    return result;
}

static DrvBq79616Result startAdc(uint8_t device_address, bool is_low_pass_filter_enabled,
                                 bool is_continuous) {
    if (!isValidDeviceAddress(device_address)) {
        return DRV_BQ79616_RESULT_INVALID_ARG;
    }

    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;
    AdcCtrl1 reg = {.lpf_vcell_en = is_low_pass_filter_enabled,
                    .main_go = 1,
                    .main_mode = is_continuous ? 2 : 1};

    result = deviceWrite(device_address, ADC_CTRL1, (uint8_t*)&reg, sizeof(reg));

    return result;
}

static DrvBq79616Result stopAdc(uint8_t device_address) {
    if (!isValidDeviceAddress(device_address)) {
        return DRV_BQ79616_RESULT_INVALID_ARG;
    }

    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;
    AdcCtrl1 reg = {
        .main_go = 0,
    };

    result = deviceWrite(device_address, ADC_CTRL1, (uint8_t*)&reg, sizeof(reg));

    return result;
}

DrvBq79616Result startBalancing(uint8_t device_address, DrvBq79616BalancingConfig* config) {
    if (!isValidDeviceAddress(device_address) || (config == NULL) ||
        (config->active_cell_count > MAX_CELL_COUNT)) {
        return DRV_BQ79616_RESULT_INVALID_ARG;
    }

    if (((config->threshold_voltage < 2.45) || (config->threshold_voltage > 4.0)) &&
        config->is_auto) {
        return DRV_BQ79616_RESULT_INVALID_ARG;
    }

    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;

    CbCellCtrl cb_cell_ctrl = {0};

    if (config->is_auto) {
        cb_cell_ctrl.time = (uint8_t)config->time;

        for (uint8_t i = 0; i < config->active_cell_count; ++i) {
            result = deviceWrite(device_address, CB_CELL1_CTRL - i, (uint8_t*)&cb_cell_ctrl,
                                 sizeof(cb_cell_ctrl));
            if (result != DRV_BQ79616_RESULT_OK) {
                return DRV_BQ79616_RESULT_FAIL;
            }
        }
    } else {
        for (uint8_t i = 0; i < MAX_CELL_COUNT; ++i) {
            if (config->is_manual_target_enabled[i]) {
                cb_cell_ctrl.time = (uint8_t)config->time;
            } else {
                cb_cell_ctrl.time = (uint8_t)DRV_BQ79616_BALANCING_TIME_STOP;
            }

            result = deviceWrite(device_address, CB_CELL1_CTRL - i, (uint8_t*)&cb_cell_ctrl,
                                 sizeof(cb_cell_ctrl));
            if (result != DRV_BQ79616_RESULT_OK) {
                return DRV_BQ79616_RESULT_FAIL;
            }
        }
    }

    OvUvCtrl ovuv_ctrl = {0};
    VcbDoneThresh vcb_done_thresh = {0};
    BalCtrl1 bal_ctrl1 = {.duty = 0};
    BalCtrl2 bal_ctrl2 = {.bal_go = 1, .auto_bal = config->is_auto ? 1 : 0};

    uint8_t* regs[] = {(uint8_t*)&vcb_done_thresh, (uint8_t*)&ovuv_ctrl, (uint8_t*)&bal_ctrl1,
                       (uint8_t*)&bal_ctrl2};
    uint16_t reg_addresses[] = {VCB_DONE_THRESH, OVUV_CTRL, BAL_CTRL1, BAL_CTRL2};

    if (config->is_auto) {
        if (config->threshold_voltage < 2.45) {
            vcb_done_thresh.cb_thr = 0x01;
        } else if (config->threshold_voltage > 4.0) {
            vcb_done_thresh.cb_thr = 0x3F;
        } else {
            vcb_done_thresh.cb_thr = (uint8_t)((config->threshold_voltage - 2.45) / 0.025);
        }
    } else {
        vcb_done_thresh.cb_thr = 0x01;
    }

    result = deviceRead(device_address, OVUV_CTRL, (uint8_t*)&ovuv_ctrl, sizeof(OVUV_CTRL));
    if (result != DRV_BQ79616_RESULT_OK) {
        return DRV_BQ79616_RESULT_FAIL;
    }

    ovuv_ctrl.ovuv_go = 1;

    for (uint8_t i = 0; i < sizeof(regs) / sizeof(regs[0]); ++i) {
        result = deviceWrite(device_address, reg_addresses[i], regs[i], sizeof(uint8_t));
        if (result != DRV_BQ79616_RESULT_OK) {
            return DRV_BQ79616_RESULT_FAIL;
        }
    }

    return result;
}

DrvBq79616Result stopBalancing(uint8_t device_address) {
    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;

    CbCellCtrl cb_cell_ctrl = {.time = 0};
    BalCtrl2 bal_ctrl2 = {.bal_go = 1, .auto_bal = 0};

    for (uint8_t i = 0; i < MAX_CELL_COUNT; ++i) {
        result = deviceWrite(device_address, CB_CELL1_CTRL - i, (uint8_t*)&cb_cell_ctrl,
                             sizeof(cb_cell_ctrl));
        if (result != DRV_BQ79616_RESULT_OK) {
            return DRV_BQ79616_RESULT_FAIL;
        }
    }

    result = deviceWrite(device_address, BAL_CTRL2, (uint8_t*)&bal_ctrl2, sizeof(uint8_t));
    if (result != DRV_BQ79616_RESULT_OK) {
        return DRV_BQ79616_RESULT_FAIL;
    }

    return result;
}

static DrvBq79616Result getMainAdcValue(uint8_t device_address, DrvBq79616MainAdcValue* value) {
    if (!isValidDeviceAddress(device_address) || (value == NULL)) {
        return DRV_BQ79616_RESULT_INVALID_ARG;
    }

    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;
    uint8_t vcell[VCELL1_LO - VCELL16_HI + 1] = {0};
    uint8_t die_temp1[DIETEMP1_LO - DIETEMP1_HI + 1] = {0};
    uint8_t busbar[BUSBAR_LO - BUSBAR_HI + 1] = {0};
    uint8_t tsref[TSREF_LO - TSREF_HI + 1] = {0};
    uint8_t gpio[GPIO8_LO - GPIO1_HI + 1] = {0};

    result = deviceRead(device_address, VCELL16_HI, vcell, sizeof(vcell));
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    result = deviceRead(device_address, DIETEMP1_HI, die_temp1, sizeof(die_temp1));
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    result = deviceRead(device_address, BUSBAR_HI, busbar, sizeof(busbar));
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    result = deviceRead(device_address, TSREF_HI, tsref, sizeof(tsref));
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    result = deviceRead(device_address, GPIO1_HI, gpio, sizeof(gpio));
    if (result != DRV_BQ79616_RESULT_OK) {
        return result;
    }

    for (uint8_t i = 0; i < MAX_CELL_COUNT; ++i) {
        uint16_t vcell_value = ((uint16_t)vcell[(2 * i)] << 8) | vcell[(2 * i) + 1];

        value->cell_voltages[MAX_CELL_COUNT - 1 - i] = toCellVoltage(vcell_value);
        if (value->cell_voltages[MAX_CELL_COUNT - 1 - i] < 0) {
            value->cell_voltages[MAX_CELL_COUNT - 1 - i] = 0;
        }
    }

    uint16_t die_temp_value = ((uint16_t)die_temp1[0] << 8) | die_temp1[1];
    value->die_temperature = toTemperature(die_temp_value);

    uint16_t busbar_value = ((uint16_t)busbar[0] << 8) | busbar[1];
    value->busbar_voltage = toCellVoltage(busbar_value);

    uint16_t tsref_value = ((uint16_t)tsref[0] << 8) | tsref[1];
    value->tsref_voltage = toTsRefVoltage(tsref_value);

    for (uint8_t i = 0; i < MAX_GPIO_COUNT; ++i) {
        uint16_t gpio_value = ((uint16_t)gpio[(2 * i)] << 8) | gpio[(2 * i) + 1];
        value->gpio_voltages[i] = toGpioVoltage(gpio_value);
    }

    return result;
}

static DrvBq79616Result getStatus(uint8_t device_address, DrvBq79616Status* status) {
    if (!isValidDeviceAddress(device_address) || (status == NULL)) {
        return DRV_BQ79616_RESULT_INVALID_ARG;
    }

    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;
    BalStat bal_stat = {0};
    DevStat dev_stat = {0};
    CbComplete cb_complete1 = {0};
    CbComplete cb_complete2 = {0};

    uint8_t* regs[] = {(uint8_t*)&bal_stat, (uint8_t*)&dev_stat, (uint8_t*)&cb_complete1,
                       (uint8_t*)&cb_complete2};
    uint16_t reg_addresses[] = {BAL_STAT, DEV_STAT, CB_COMPLETE1, CB_COMPLETE2};

    for (uint8_t i = 0; i < sizeof(regs) / sizeof(regs[0]); ++i) {
        result = deviceRead(device_address, reg_addresses[i], regs[i], sizeof(uint8_t));
        if (result != DRV_BQ79616_RESULT_OK) {
            return DRV_BQ79616_RESULT_FAIL;
        }
    }

    BalCtrl3 bal_ctrl3 = {.bal_time_go = 1};
    BalTime bal_time = {0};

    for (uint8_t i = 0; i < MAX_CELL_COUNT; ++i) {
        bal_ctrl3.bal_time_sel = i;

        result = deviceWrite(device_address, BAL_CTRL3, (uint8_t*)&bal_ctrl3, sizeof(bal_ctrl3));
        if (result != DRV_BQ79616_RESULT_OK) {
            return DRV_BQ79616_RESULT_FAIL;
        }

        result = deviceRead(device_address, BAL_TIME, (uint8_t*)&bal_time, sizeof(bal_time));
        if (result != DRV_BQ79616_RESULT_OK) {
            return DRV_BQ79616_RESULT_FAIL;
        }

        status->balancing_remaining_time[i] =
            bal_time.time_unit ? bal_time.time * 5 * 60 : bal_time.time * 5;
    }

    status->is_balancing_done[0] = cb_complete2.cell1_done;
    status->is_balancing_done[1] = cb_complete2.cell2_done;
    status->is_balancing_done[2] = cb_complete2.cell3_done;
    status->is_balancing_done[3] = cb_complete2.cell4_done;
    status->is_balancing_done[4] = cb_complete2.cell5_done;
    status->is_balancing_done[5] = cb_complete2.cell6_done;
    status->is_balancing_done[6] = cb_complete2.cell7_done;
    status->is_balancing_done[7] = cb_complete2.cell8_done;
    status->is_balancing_done[8] = cb_complete1.cell1_done;
    status->is_balancing_done[9] = cb_complete1.cell2_done;
    status->is_balancing_done[10] = cb_complete1.cell3_done;
    status->is_balancing_done[11] = cb_complete1.cell4_done;
    status->is_balancing_done[11] = cb_complete1.cell5_done;
    status->is_balancing_done[13] = cb_complete1.cell6_done;
    status->is_balancing_done[14] = cb_complete1.cell7_done;
    status->is_balancing_done[15] = cb_complete1.cell8_done;

    status->is_balancing_running = bal_stat.cb_run;
    status->is_balancing_pause = bal_stat.cb_inpause;
    status->is_balancing_fault = bal_stat.invalid_cbconf;
    status->is_balancing_over_temperature_detected = bal_stat.ot_pause_det;

    status->is_main_adc_running = dev_stat.main_run;
    status->is_auxiliary_adc_running = dev_stat.aux_run;
    status->is_voltage_protector_running = dev_stat.ovuv_run;
    status->is_temperature_protector_running = dev_stat.otut_run;

    return result;
}

// memory mapping
//=============================================================================================================================

const DrvBq79616 drv_bq79616 = {
    .initialize = initialize,
    .deinitialize = deinitialize,
    .ping = ping,
    .setConfiguration = setConfiguration,
    .getConfiguration = getConfiguration,
    .startAdc = startAdc,
    .stopAdc = stopAdc,
    .startBalancing = startBalancing,
    .stopBalancing = stopBalancing,
    .getMainAdcValue = getMainAdcValue,
    .getStatus = getStatus,
    .private = &drv_bq79616_private,
};
