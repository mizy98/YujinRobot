#ifndef DRV_MCP7941X_DEFINE_H
#define DRV_MCP7941X_DEFINE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)

typedef enum {
    DRV_MCP7941X_OUTPUT_MODE_SQUARE_WAVE = 0,
    DRV_MCP7941X_OUTPUT_MODE_ALARM_PULSE = 1,
    DRV_MCP7941X_OUTPUT_MODE_GENERAL_PURPOSE = 2,
} DrvMcp7941xOutputMode;

typedef struct {
    int (*write)(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                 const uint16_t length);
    int (*read)(uint8_t device_address, uint8_t register_address, uint8_t* data,
                const uint16_t length);
    void (*delayMs)(uint32_t ms);
} DrvMcp7941xInterface;

typedef struct {
    DrvMcp7941xOutputMode mode;
    bool external_oscillator_enabled : 1;
    bool alarm0_enabled : 1;
    bool alarm1_enabled : 1;
} DrvMcp7941xConfig;

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day_of_week;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} DrvMcp7941xDateTime;

typedef struct {
    bool is_oscillator_running : 1;
    bool power_fail_detected : 1;
} DrvMcp7941xStatus;

typedef struct {
    uint32_t size;       // bytes
    uint32_t page_size;  // bytes
} DrvMcp7941xSramInfo;

typedef struct {
    uint32_t size;       // bytes
    uint32_t page_size;  // bytes
} DrvMcp7941xEepromInfo;

#pragma pack(pop)

#endif