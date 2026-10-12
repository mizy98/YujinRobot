#include "ui_protocol.h"

#include <string.h>

#include "../interface/rtos.h"
#include "../interface/system.h"
#include "../interface/ui_comm.h"
#include "crc16_modbus.h"

#define self (ui_protocol_private)

/*
 *  ┌─────────────┬─────────────┬──────────┬───────────┬─────────────────────┬─────────────┐
 *  │   Header    │   Length    │ Command  │ Sequence  │       Payload       │   CRC-16    │
 *  │   (2 byte)  │   (2 byte)  │ (1 byte) │ (1 byte)  │    (0~2042 byte)    │   (2 byte)  │
 *  └─────────────┴─────────────┴──────────┴───────────┴─────────────────────┴─────────────┘
 *
 *  Header   : 0xA5, 0x5A (fixed value)
 *  Length   : Command + Sequence + Payload length (Little Endian)
 *  Command  : Command code (0x01~0xFF)
 *  Sequence : Packet sequence number (0~255, cyclic)
 *  Payload  : Data (variable length, up to 2042 bytes)
 *  CRC-16   : CRC-16 MODBUS (Little Endian), calculated over Length, Command, Sequence, and Payload
 *
 *  Example: "Hello" transmission packet
 *  ┌────┬────┬────┬────┬────┬────┬───┬───┬───┬───┬───┬────┬────┐
 *  │A5  │5A  │07  │00  │01  │33  │48 │65 │6C │6C │6F │XX  │XX  │
 *  └────┴────┴────┴────┴────┴────┴───┴───┴───┴───┴───┴────┴────┘
 *    Header    Length Command   Seq    Payload Data     CRC-16
 *   (0xA55A)    (7)   (0x01)    (51)      "Hello"
 */

#define RX_BUFFER_SIZE (2048u)
#define TX_BUFFER_SIZE (2048u + 512u)

#define RX_BUFFER_MASK (RX_BUFFER_SIZE - 1u)

#define MAX_DATA_LOG_ENTRY_REQUEST_COUNT (31u)

#define PROTOCOL_HEADER (0xA55A)
#define PROTOCOL_HEADER_SIZE (2u)
#define PROTOCOL_LENGTH_SIZE (2u)
#define PROTOCOL_COMMAND_SIZE (1u)
#define PROTOCOL_SEQUENCE_SIZE (1u)
#define PROTOCOL_CRC_SIZE (2u)
#define PROTOCOL_MIN_SIZE                                                  \
    (PROTOCOL_HEADER_SIZE + PROTOCOL_LENGTH_SIZE + PROTOCOL_COMMAND_SIZE + \
     PROTOCOL_SEQUENCE_SIZE + PROTOCOL_CRC_SIZE)
#define PROTOCOL_MAX_PAYLOAD (2048u)

typedef enum {
    PARSE_STATE_IDLE,
    PARSE_STATE_HEADER,
    PARSE_STATE_LENGTH,
    PARSE_STATE_COMMAND,
    PARSE_STATE_SEQUENCE,
    PARSE_STATE_PAYLOAD,
    PARSE_STATE_CRC
} ParseState;

typedef struct {
    uint16_t header;
    uint16_t length;
    uint8_t command;
    uint8_t sequence;
    uint8_t payload[PROTOCOL_MAX_PAYLOAD];
    uint16_t crc;  // CRC-16 MODBUS
} __attribute__((packed)) ProtocolPacket;

static struct {
    AppErrorInfo error_info;

    uint8_t tx_buffer[TX_BUFFER_SIZE];
    uint8_t rx_buffer[RX_BUFFER_SIZE];
    uint16_t rx_buffer_head;
    uint16_t rx_buffer_tail;

    ParseState parse_state;
    ProtocolPacket current_packet;
    uint16_t parse_index;
    uint16_t expected_length;
    uint16_t parse_length;
    uint8_t sequence;

    UiProtocolCallback callback;
} ui_protocol_private;

static inline uint16_t _rxBufferCount(void) {
    return (self.rx_buffer_head - self.rx_buffer_tail) & RX_BUFFER_MASK;
}

static inline uint16_t _rxBufferSpace(void) { return RX_BUFFER_SIZE - _rxBufferCount() - 1u; }

static inline bool _rxBufferFull(void) { return _rxBufferSpace() == 0u; }

static inline bool _rxBufferEmpty(void) { return self.rx_buffer_head == self.rx_buffer_tail; }

static inline void _resetParseState(void) {
    self.parse_state = PARSE_STATE_IDLE;
    self.parse_index = 0;
    self.expected_length = 0;
}

static void _onDataReceived(const uint8_t* data, uint16_t size) {
    if (size == 0 || data == NULL) {
        return;
    }

    if (_rxBufferSpace() < size) {
        uint16_t overflow = size - _rxBufferSpace();
        self.rx_buffer_tail = (self.rx_buffer_tail + overflow) & RX_BUFFER_MASK;
    }

    uint16_t bytes_to_end = RX_BUFFER_SIZE - self.rx_buffer_head;

    if (size <= bytes_to_end) {
        memcpy(&self.rx_buffer[self.rx_buffer_head], data, size);
        self.rx_buffer_head = (self.rx_buffer_head + size) & RX_BUFFER_MASK;
    } else {
        memcpy(&self.rx_buffer[self.rx_buffer_head], data, bytes_to_end);
        uint16_t remaining = size - bytes_to_end;
        memcpy(&self.rx_buffer[0], data + bytes_to_end, remaining);
        self.rx_buffer_head = remaining;
    }
}

static AppResult _sendPacket(uint8_t command, uint8_t sequence, const uint8_t* payload,
                             uint16_t payload_length) {
    if (payload_length > PROTOCOL_MAX_PAYLOAD) {
        return APP_RESULT_FAIL;
    }

    uint8_t* packet_buffer = self.tx_buffer;
    uint16_t packet_index = 0;

    // Header (Little Endian)
    packet_buffer[packet_index++] = (uint8_t)(PROTOCOL_HEADER >> 8);
    packet_buffer[packet_index++] = (uint8_t)(PROTOCOL_HEADER & 0xFF);

    // Length = Command + Sequence + Payload (Little Endian)
    uint16_t length = PROTOCOL_COMMAND_SIZE + PROTOCOL_SEQUENCE_SIZE + payload_length;
    packet_buffer[packet_index++] = (uint8_t)(length & 0xFF);
    packet_buffer[packet_index++] = (uint8_t)(length >> 8);

    // Command
    packet_buffer[packet_index++] = command;

    // Sequence
    packet_buffer[packet_index++] = sequence;

    // Payload
    if (payload_length > 0 && payload != NULL) {
        memcpy(&packet_buffer[packet_index], payload, payload_length);
        packet_index += payload_length;
    }

    // CRC-16 (exclude header)
    uint16_t crc =
        Crc16Modbus_Calculate(&packet_buffer[PROTOCOL_HEADER_SIZE], length + PROTOCOL_LENGTH_SIZE);
    packet_buffer[packet_index++] = (uint8_t)(crc & 0xFF);
    packet_buffer[packet_index++] = (uint8_t)(crc >> 8);

    self.sequence = sequence;

    return UiComm_SendData(packet_buffer, packet_index);
}

AppResult UiProtocol_Initialize(UiProtocolCallback* callback) {
    if (callback == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_PROTOCOL_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    self.callback = *callback;

    UiCommCallback ui_comm_callback = {
        .onDataReceived = _onDataReceived,
    };

    self.rx_buffer_head = 0;
    self.rx_buffer_tail = 0;

    _resetParseState();

    self.sequence = 0;

    self.error_info = (AppErrorInfo){0};

    if (UiComm_Initialize(&ui_comm_callback) != APP_RESULT_OK) {
        UiComm_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult UiProtocol_Deinitialize() {
    if (UiComm_Deinitialize() != APP_RESULT_OK) {
        UiComm_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void UiProtocol_GetLastErrorInfo(AppErrorInfo* error_info) {
    if (error_info != NULL) {
        *error_info = self.error_info;
    }
}

void UiProtocol_Process() {
    UiComm_Process();
    UiProtocol_Parse();
}

void UiProtocol_Parse() {
    uint16_t count = _rxBufferCount();
    if (count == 0) {
        return;
    }

    for (uint16_t i = 0; i < count; i++) {
        uint8_t byte = self.rx_buffer[self.rx_buffer_tail];
        self.rx_buffer_tail = (self.rx_buffer_tail + 1) & RX_BUFFER_MASK;

        switch (self.parse_state) {
            case PARSE_STATE_IDLE:
                if (byte == 0xA5) {
                    self.current_packet.header = byte;
                    self.parse_state = PARSE_STATE_HEADER;
                    self.parse_index = 1;
                }
                break;

            case PARSE_STATE_HEADER:
                if (byte == 0x5A && self.parse_index == 1) {
                    self.current_packet.header |= (uint16_t)byte << 8;
                    self.parse_state = PARSE_STATE_LENGTH;
                    self.parse_index = 0;
                } else {
                    _resetParseState();
                    if (byte == 0xA5) {
                        self.current_packet.header = byte;
                        self.parse_state = PARSE_STATE_HEADER;
                        self.parse_index = 1;
                    }
                }
                break;

            case PARSE_STATE_LENGTH:
                if (self.parse_index == 0) {
                    self.current_packet.length = byte;
                    self.parse_index = 1;
                } else if (self.parse_index == 1) {
                    self.current_packet.length |= (uint16_t)byte << 8;

                    if (self.current_packet.length <
                            (PROTOCOL_COMMAND_SIZE + PROTOCOL_SEQUENCE_SIZE) ||
                        self.current_packet.length >
                            (PROTOCOL_COMMAND_SIZE + PROTOCOL_SEQUENCE_SIZE +
                             PROTOCOL_MAX_PAYLOAD)) {
                        _resetParseState();
                        break;
                    }

                    self.expected_length = self.current_packet.length;
                    self.parse_state = PARSE_STATE_COMMAND;
                    self.parse_index = 0;
                }
                break;

            case PARSE_STATE_COMMAND:
                self.current_packet.command = byte;
                self.parse_state = PARSE_STATE_SEQUENCE;
                break;

            case PARSE_STATE_SEQUENCE:
                self.current_packet.sequence = byte;

                self.parse_length =
                    self.expected_length - PROTOCOL_COMMAND_SIZE - PROTOCOL_SEQUENCE_SIZE;

                if (self.parse_length > 0) {
                    self.parse_state = PARSE_STATE_PAYLOAD;
                    self.parse_index = 0;
                } else {
                    self.parse_state = PARSE_STATE_CRC;
                    self.parse_index = 0;
                }
                break;

            case PARSE_STATE_PAYLOAD:
                self.parse_length =
                    self.expected_length - PROTOCOL_COMMAND_SIZE - PROTOCOL_SEQUENCE_SIZE;
                if (self.parse_index < self.parse_length &&
                    self.parse_index < PROTOCOL_MAX_PAYLOAD) {
                    self.current_packet.payload[self.parse_index] = byte;
                    self.parse_index++;

                    if (self.parse_index >= self.parse_length) {
                        self.parse_state = PARSE_STATE_CRC;
                        self.parse_index = 0;
                    }
                } else {
                    _resetParseState();
                }
                break;

            case PARSE_STATE_CRC:
                if (self.parse_index == 0) {
                    self.current_packet.crc = byte;
                    self.parse_index = 1;
                } else if (self.parse_index == 1) {
                    self.current_packet.crc |= (uint16_t)byte << 8;

                    uint16_t calculated_crc =
                        Crc16Modbus_Calculate((uint8_t*)&self.current_packet.length,
                                              self.expected_length + PROTOCOL_LENGTH_SIZE);

                    if (calculated_crc == self.current_packet.crc) {
                        if (self.callback.onCommandReceived) {
                            self.callback.onCommandReceived(
                                self.current_packet.command, self.current_packet.sequence,
                                self.current_packet.payload, self.parse_length);
                        }
                    }

                    _resetParseState();
                }
                break;

            default:
                _resetParseState();
                break;
        }
    }
}

AppResult UiProtocol_SendDataWithSequence(UiProtocolCommand command, uint8_t sequence,
                                          const uint8_t* data, uint16_t size) {
    return _sendPacket(command, sequence, data, size);
}

AppResult UiProtocol_SendData(UiProtocolCommand command, const uint8_t* data, uint16_t size) {
    uint8_t sequence = self.sequence++;
    if (self.sequence > 255) {
        self.sequence = 0;
    }
    return _sendPacket(command, sequence, data, size);
}
