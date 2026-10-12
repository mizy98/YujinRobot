#ifndef COMM_HGP_DEFINE_H
#define COMM_HGP_DEFINE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define COMM_HGP_OK (0)
#define COMM_HGP_FAIL (-1)

#define HGP_CAN_BASE_ID (0x400)

typedef enum {
    COMM_HGP_ERROR_NONE = 0,
    COMM_HGP_ERROR_TIMEOUT = 0x01,
    COMM_HGP_ERROR_CHECKSUM = 0x02,
    COMM_HGP_ERROR_RESPONSE = 0x03,
} CommHgpErrorCode;

typedef enum {
    HGP_SERIAL_COMMAND_REQUEST_STATUS = 0x01,
    HGP_SERIAL_COMMAND_OPERATE = 0x02,
    HGP_SERIAL_COMMAND_RESPONSE_STATUS = 0x03,
    HGP_SERIAL_COMMAND_RESPONSE_ERROR = 0x1F,
} HgpSerialCommand;

#pragma pack(push, 1)

typedef struct {
    uint8_t device_address;
} CommHgpConfig;

typedef union {
    struct {
        uint16_t ovp : 1;
        uint16_t uvp : 1;
        uint16_t cocp : 1;
        uint16_t docp : 1;
        uint16_t otp : 1;
        uint16_t utp : 1;
        uint16_t bmu_error : 1;
        uint16_t reserved : 9;
    };
    uint16_t value;
} CommHgpStateData;

typedef struct {
    uint16_t pack_voltage;              // 0.01V
    int16_t pack_current;               // 0.01A, +: charge, -: discharge
    uint16_t soc;                       // %
    CommHgpStateData state;             // @ref CommHgpStateData
    uint16_t charge_remaining_time;     // minutes
    uint16_t discharge_remaining_time;  // minutes
    int16_t temperature;                // 0.1°C
    uint16_t soh;                       // %
    uint16_t remaining_capacity;        // 0.01Ah
    uint16_t remaining_energy;          // 0.1Wh
} CommHgpData;

typedef struct {
    int (*write)(const uint8_t* data, uint16_t length);
    void (*getStatus)(CommHgpData* status);
} CommHgpSerialInterface;

typedef struct {
    void (*onParseStarted)();
    void (*onParseCompleted)();
    void (*onCommandReceived)(uint8_t address, HgpSerialCommand command);
    void (*onErrorOccurred)(uint8_t error_code);
} CommHgpSerialCallback;

typedef struct {
    int (*write)(uint32_t id, uint8_t ide, uint8_t rtr, uint8_t* data, uint16_t length);
} CommHgpCanInterface;

typedef struct {
    void (*onDataRequested)();
    void (*onDataStreamStartRequested)();
    void (*onDataStreamStopRequested)();
} CommHgpCanCallback;

typedef struct {
} CommHgpCanData;

#pragma pack(pop)

#endif