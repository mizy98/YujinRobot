/*
 * ============================================================================
 *  HGP Serial Communication Protocol
 * ============================================================================
 *
 *  [Frame Layout]
 *
 *   Byte  0     1     2      3      4     5      6 ~ 5+N   6+N    7+N   8+N
 *        +-----+-----+------+------+-----+-------+---------+------+-----+-----+
 *        | 0xAF| 0xFA| ADDR |  LEN | CMD | ORDER | DATA(N) | CSUM | 0xAF| 0xA0|
 *        +-----+-----+------+------+-----+-------+---------+------+-----+-----+
 *         <---START--->                                             <---END--->
 *
 *   LEN  = CMD(1) + ORDER(1) + DATA(N) + CSUM(1) = N + 3    min=3, max=23
 *   CSUM = ADDR + LEN + CMD + ORDER + DATA[0] + ... + DATA[N-1]
 *
 * ============================================================================
 *  [Command Set]
 *
 *   CMD   Constant              Direction    Description
 *   ----  --------------------  -----------  -------------------------
 *   0x01  REQUEST_STATUS        Host -> BMS  Request BMS status data
 *   0x02  OPERATE               Host -> BMS  Send operation command
 *   0x03  RESPONSE_STATUS       BMS -> Host  Return requested status
 *   0x1F  RESPONSE_ERROR        BMS -> Host  Error notification
 *
 * ============================================================================
 *  [REQUEST_STATUS (CMD=0x01) - Data Byte Flags]
 *
 *   DATA[0] = RequestData1                  DATA[1] = RequestData2
 *   +-bit-+-------------------------+        +-bit-+-------------------------+
 *   |  7  | (reserved)              |        |  7  | (reserved)              |
 *   |  6  | temperature             |        |  6  | (reserved)              |
 *   |  5  | discharge_remaining_time|        |  5  | (reserved)              |
 *   |  4  | charge_remaining_time   |        |  4  | (reserved)              |
 *   |  3  | state                   |        |  3  | (reserved)              |
 *   |  2  | soc                     |        |  2  | remaining_energy        |
 *   |  1  | current                 |        |  1  | remaining_capacity      |
 *   |  0  | pack_voltage            |        |  0  | soh                     |
 *   +-----+-------------------------+        +-----+-------------------------+
 *
 * ============================================================================
 *  [RESPONSE_STATUS (CMD=0x03) - Data Bytes (Big-Endian, 2B each)]
 *
 *   Fields are included only if the corresponding request bit was set,
 *   in order of bit position (LSB first):
 *
 *   +-------------------------+------+
 *   | Field                   | Size |
 *   +-------------------------+------+
 *   | pack_voltage            |  2B  |
 *   | current                 |  2B  |
 *   | soc                     |  2B  |
 *   | state                   |  2B  |
 *   | charge_remaining_time   |  2B  |
 *   | discharge_remaining_time|  2B  |
 *   | temperature             |  2B  |
 *   | soh                     |  2B  |
 *   | remaining_capacity      |  2B  |
 *   | remaining_energy        |  2B  |
 *   +-------------------------+------+
 *   (max DATA = 20 bytes when all 10 fields requested)
 *
 * ============================================================================
 *  [Parse State Machine]
 *
 *           byte == 0xAF       byte == 0xFA     byte == ADDR      3 <= byte <= 23
 *              +------+           +------+          +------+           +------+
 *   WAIT_START_1 ---> WAIT_START_2 ---> READ_ADDRESS ---> READ_LENGTH ---> READ_ORDER
 *       ^                  |                  |                |                |
 *       |               [else]             [else]           [else]       data_len > 0?
 *       |                reset              reset            reset              |
 *       |                                                              YES      |  NO
 *       |                                                               v       v
 *       |                                                          READ_DATA  READ_CHECKSUM
 *       |                                                          [N bytes]      |
 *       |                                                               |         |
 *       |                                                               +---------+
 *       |                                                                    |
 *       |                                                         CSUM ok?  CSUM fail?
 *       |                                                              |         |
 *       |                                                         WAIT_END_1   reset
 *       |                                                              |      + onErrorOccurred
 *       |                                          byte == 0xAF       |
 *       |                                              +------+        |
 *       |                                         WAIT_END_2 <--------+
 *       |                                              |
 *       |                          byte == 0xA0        |
 *       |                      processCommand() -------+
 *       |                              |
 *       +------------- reset() --------+   (always called after END_2)
 *
 * ============================================================================
 */

#include "comm_hgp_serial.h"

#define self (comm_hgp_serial_private)

#define MAX_DATA_LENGTH (20u)
#define MAX_TX_BUFFER_SIZE (128u)

#define START_SENTENCE_1 (0xAF)
#define START_SENTENCE_2 (0xFA)

#define END_SENTENCE_1 (0xAF)
#define END_SENTENCE_2 (0xA0)

#define TIMEOUT_INTERVAL (1000u)

typedef enum {
    PARSE_STATE_WAIT_START_1,
    PARSE_STATE_WAIT_START_2,
    PARSE_STATE_READ_ADDRESS,
    PARSE_STATE_READ_LENGTH,
    PARSE_STATE_READ_COMMAND,
    PARSE_STATE_READ_ORDER,
    PARSE_STATE_READ_DATA,
    PARSE_STATE_READ_CHECKSUM,
    PARSE_STATE_WAIT_END_1,
    PARSE_STATE_WAIT_END_2,
} ParseState;

#pragma pack(push, 1)
typedef struct {
    uint8_t start_sentence_1;
    uint8_t start_sentence_2;
    uint8_t address;
    uint8_t length;  // = command (1) + order (1) + data (N) + checksum (1) = N + 3
    uint8_t command;
    uint8_t order;
    uint8_t data[MAX_DATA_LENGTH];
    uint8_t checksum;
    uint8_t end_sentence_1;
    uint8_t end_sentence_2;

    // For parsing
    uint8_t data_length;
    uint8_t data_index;
} DataFrame;

typedef union {
    struct {
        uint8_t pack_voltage : 1;
        uint8_t current : 1;
        uint8_t soc : 1;
        uint8_t state : 1;
        uint8_t charge_remaining_time : 1;
        uint8_t discharge_remaining_time : 1;
        uint8_t temperature : 1;
        uint8_t unused : 1;
    };
    uint8_t value;
} RequestData1;

typedef union {
    struct {
        uint8_t soh : 1;
        uint8_t remaining_capacity : 1;
        uint8_t remaining_energy : 1;
        uint8_t unused0 : 1;
        uint8_t unused1 : 1;
        uint8_t unused2 : 1;
        uint8_t unused3 : 1;
        uint8_t unused4 : 1;
    };
    uint8_t value;
} RequestData2;

static struct {
    ParseState parse_state;
    DataFrame current_frame;

    uint8_t tx_buffer[MAX_TX_BUFFER_SIZE];

    uint8_t device_address;

    CommHgpSerialInterface interface;
    CommHgpSerialCallback callback;
} comm_hgp_serial_private;

#pragma pack(pop)

static uint8_t _calculateFrameChecksum(const DataFrame* frame) {
    uint8_t checksum = 0;

    checksum += frame->address;
    checksum += frame->length;
    checksum += frame->command;
    checksum += frame->order;

    for (uint8_t i = 0; i < frame->data_length; i++) {
        checksum += frame->data[i];
    }

    return checksum;
}

static uint8_t _calculateChecksum(uint8_t* data, uint16_t length) {
    uint8_t checksum = 0;

    for (uint16_t i = 0; i < length; i++) {
        checksum += data[i];
    }

    return checksum;
}

static void _resetParseState() {
    comm_hgp_serial_private.parse_state = PARSE_STATE_WAIT_START_1;

    self.callback.onParseCompleted();
}

static int _processCommand(const DataFrame* frame) {
    switch (frame->command) {
        case HGP_SERIAL_COMMAND_REQUEST_STATUS: {
            CommHgpData status = {0};
            self.interface.getStatus(&status);

            uint16_t index = 0;
            uint16_t data_length = 0;
            RequestData1* request1 = (RequestData1*)&frame->data[0];
            RequestData2* request2 = (RequestData2*)&frame->data[1];

            self.tx_buffer[index++] = START_SENTENCE_1;
            self.tx_buffer[index++] = START_SENTENCE_2;
            self.tx_buffer[index++] = self.device_address;
            self.tx_buffer[index++] = 0;  // placeholder for length
            self.tx_buffer[index++] = HGP_SERIAL_COMMAND_RESPONSE_STATUS;
            self.tx_buffer[index++] = self.device_address;  // echo back the order (device address)

            if (request1->pack_voltage > 0) {
                self.tx_buffer[index++] = (status.pack_voltage >> 8) & 0xFF;
                self.tx_buffer[index++] = status.pack_voltage & 0xFF;
                data_length += 2;
            }

            if (request1->current) {
                self.tx_buffer[index++] = (status.pack_current >> 8) & 0xFF;
                self.tx_buffer[index++] = status.pack_current & 0xFF;
                data_length += 2;
            }

            if (request1->soc > 0) {
                self.tx_buffer[index++] = (status.soc >> 8) & 0xFF;
                self.tx_buffer[index++] = status.soc & 0xFF;
                data_length += 2;
            }

            if (request1->state > 0) {
                self.tx_buffer[index++] = (status.state.value >> 8) & 0xFF;
                self.tx_buffer[index++] = status.state.value & 0xFF;
                data_length += 2;
            }

            if (request1->charge_remaining_time > 0) {
                self.tx_buffer[index++] = (status.charge_remaining_time >> 8) & 0xFF;
                self.tx_buffer[index++] = status.charge_remaining_time & 0xFF;
                data_length += 2;
            }

            if (request1->discharge_remaining_time > 0) {
                self.tx_buffer[index++] = (status.discharge_remaining_time >> 8) & 0xFF;
                self.tx_buffer[index++] = status.discharge_remaining_time & 0xFF;
                data_length += 2;
            }

            if (request1->temperature > 0) {
                self.tx_buffer[index++] = (status.temperature >> 8) & 0xFF;
                self.tx_buffer[index++] = status.temperature & 0xFF;
                data_length += 2;
            }

            if (request1->unused > 0) {
                self.tx_buffer[index++] = 0;
                self.tx_buffer[index++] = 0;
                data_length += 2;
            }

            if (request2->soh > 0) {
                self.tx_buffer[index++] = (status.soh >> 8) & 0xFF;
                self.tx_buffer[index++] = status.soh & 0xFF;
                data_length += 2;
            }

            if (request2->remaining_capacity > 0) {
                self.tx_buffer[index++] = (status.remaining_capacity >> 8) & 0xFF;
                self.tx_buffer[index++] = status.remaining_capacity & 0xFF;
                data_length += 2;
            }

            if (request2->remaining_energy > 0) {
                self.tx_buffer[index++] = (status.remaining_energy >> 8) & 0xFF;
                self.tx_buffer[index++] = status.remaining_energy & 0xFF;
                data_length += 2;
            }

            if (request2->unused0 > 0) {
                self.tx_buffer[index++] = 0;
                self.tx_buffer[index++] = 0;
                data_length += 2;
            }

            if (request2->unused1 > 0) {
                self.tx_buffer[index++] = 0;
                self.tx_buffer[index++] = 0;
                data_length += 2;
            }

            if (request2->unused2 > 0) {
                self.tx_buffer[index++] = 0;
                self.tx_buffer[index++] = 0;
                data_length += 2;
            }

            if (request2->unused3 > 0) {
                self.tx_buffer[index++] = 0;
                self.tx_buffer[index++] = 0;
                data_length += 2;
            }

            if (request2->unused4 > 0) {
                self.tx_buffer[index++] = 0;
                self.tx_buffer[index++] = 0;
                data_length += 2;
            }

            uint8_t checksum = _calculateChecksum(&self.tx_buffer[2], data_length + 4);

            self.tx_buffer[3] = data_length + 3;
            self.tx_buffer[index++] = checksum;
            self.tx_buffer[index++] = END_SENTENCE_1;
            self.tx_buffer[index++] = END_SENTENCE_2;

            if (self.interface.write(self.tx_buffer, index) != 0) {
                return -1;
            }

            break;
        }

        case HGP_SERIAL_COMMAND_OPERATE: {
            self.callback.onCommandReceived(frame->address, frame->command);
            break;
        }

        default: {
            break;
        }
    }

    return 0;
}

int CommHgpSerial_Initialize(const CommHgpConfig* config, const CommHgpSerialInterface* interface,
                             const CommHgpSerialCallback* callback) {
    if (interface == NULL || interface->write == NULL || interface->getStatus == NULL) {
        return COMM_HGP_FAIL;
    }

    if (callback == NULL || callback->onParseCompleted == NULL ||
        callback->onParseStarted == NULL || callback->onCommandReceived == NULL ||
        callback->onErrorOccurred == NULL) {
        return COMM_HGP_FAIL;
    }

    self.interface = *interface;
    self.callback = *callback;
    self.device_address = config->device_address;

    return COMM_HGP_OK;
}

int CommHgpSerial_Deinitialize(void) { return COMM_HGP_OK; }

int CommHgpSerial_Parse(uint8_t* data, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        uint8_t byte = data[i];

        switch (self.parse_state) {
            case PARSE_STATE_WAIT_START_1: {
                if (byte == START_SENTENCE_1) {
                    self.current_frame.start_sentence_1 = byte;
                    self.parse_state = PARSE_STATE_WAIT_START_2;

                    self.callback.onParseStarted();
                }
                break;
            }

            case PARSE_STATE_WAIT_START_2: {
                if (byte == START_SENTENCE_2) {
                    self.current_frame.start_sentence_2 = byte;
                    self.parse_state = PARSE_STATE_READ_ADDRESS;

                } else {
                    _resetParseState();
                }
                break;
            }

            case PARSE_STATE_READ_ADDRESS: {
                if (self.device_address == byte) {
                    self.current_frame.address = byte;
                    self.parse_state = PARSE_STATE_READ_LENGTH;

                } else {
                    _resetParseState();
                }
                break;
            }

            case PARSE_STATE_READ_LENGTH: {
                if (byte <= MAX_DATA_LENGTH &&
                    byte >= 3) {  // Minimum length is 3 (command + order + checksum)
                    self.current_frame.length = byte;
                    self.parse_state = PARSE_STATE_READ_COMMAND;

                } else {
                    _resetParseState();
                }
                break;
            }

            case PARSE_STATE_READ_COMMAND: {
                self.current_frame.command = byte;
                self.parse_state = PARSE_STATE_READ_ORDER;
                break;
            }

            case PARSE_STATE_READ_ORDER: {
                if (self.device_address == byte) {
                    self.current_frame.order = byte;

                    self.current_frame.data_index = 0;
                    self.current_frame.data_length =
                        self.current_frame.length - 3;  // Exclude command, order, checksum

                    if (self.current_frame.data_length > 0) {
                        self.parse_state = PARSE_STATE_READ_DATA;
                    } else {
                        self.parse_state = PARSE_STATE_READ_CHECKSUM;
                    }
                } else {
                    _resetParseState();
                }
                break;
            }

            case PARSE_STATE_READ_DATA: {
                if (self.current_frame.data_index < self.current_frame.data_length) {
                    self.current_frame.data[self.current_frame.data_index] = byte;
                    self.current_frame.data_index++;

                    if (self.current_frame.data_index == self.current_frame.data_length) {
                        self.parse_state = PARSE_STATE_READ_CHECKSUM;
                    }
                } else {
                    _resetParseState();
                }
                break;
            }

            case PARSE_STATE_READ_CHECKSUM: {
                self.current_frame.checksum = byte;

                uint8_t calculated_checksum = _calculateFrameChecksum(&self.current_frame);
                if (calculated_checksum == self.current_frame.checksum) {
                    self.parse_state = PARSE_STATE_WAIT_END_1;

                } else {
                    _resetParseState();
                    if (self.callback.onErrorOccurred) {
                        self.callback.onErrorOccurred(COMM_HGP_ERROR_CHECKSUM);
                    }
                }
                break;
            }

            case PARSE_STATE_WAIT_END_1: {
                if (byte == END_SENTENCE_1) {
                    self.current_frame.end_sentence_1 = byte;
                    self.parse_state = PARSE_STATE_WAIT_END_2;

                } else {
                    _resetParseState();
                }
                break;
            }

            case PARSE_STATE_WAIT_END_2: {
                if (byte == END_SENTENCE_2) {
                    self.current_frame.end_sentence_2 = byte;
                    _processCommand(&self.current_frame);
                }

                _resetParseState();
                break;
            }
        }
    }

    return COMM_HGP_OK;
}

void CommHgpSerial_ResetParseState(void) { _resetParseState(); }
