#ifndef DRV_LP5018_PROTOCOL_H
#define DRV_LP5018_PROTOCOL_H

#include "drv_lp5018_define.h"

#define self (drv_lp5018_private)

#define LP5018_BROADCAST_DEVICE_ADDRESS (0b0111100)
#define LP5018_BASE_DEVICE_ADDRESS (0b0101000)

typedef enum {
    DEVICE_CONFIG0 = 0x00,
    DEVICE_CONFIG1 = 0x01,
    LED_CONFIG0 = 0x02,
    BANK_BRIGHTNESS = 0x03,
    BANK_A_COLOR = 0x04,
    BANK_B_COLOR = 0x05,
    BANK_C_COLOR = 0x06,

    LED0_BRIGHTNESS = 0x07,
    LED1_BRIGHTNESS = 0x08,
    LED2_BRIGHTNESS = 0x09,
    LED3_BRIGHTNESS = 0x0A,
    LED4_BRIGHTNESS = 0x0B,
    LED5_BRIGHTNESS = 0x0C,
    LED6_BRIGHTNESS = 0x0D,
    LED7_BRIGHTNESS = 0x0E,

    OUT0_COLOR = 0x0F,
    OUT1_COLOR = 0x10,
    OUT2_COLOR = 0x11,
    OUT3_COLOR = 0x12,
    OUT4_COLOR = 0x13,
    OUT5_COLOR = 0x14,
    OUT6_COLOR = 0x15,
    OUT7_COLOR = 0x16,
    OUT8_COLOR = 0x17,
    OUT9_COLOR = 0x18,
    OUT10_COLOR = 0x19,
    OUT11_COLOR = 0x1A,
    OUT12_COLOR = 0x1B,
    OUT13_COLOR = 0x1C,
    OUT14_COLOR = 0x1D,
    OUT15_COLOR = 0x1E,
    OUT16_COLOR = 0x1F,
    OUT17_COLOR = 0x20,
    OUT18_COLOR = 0x21,
    OUT19_COLOR = 0x22,
    OUT20_COLOR = 0x23,
    OUT21_COLOR = 0x24,
    OUT22_COLOR = 0x25,
    OUT23_COLOR = 0x26,

    RESET = 0x27,
} DrvLp5018Register;

typedef union {
    struct {
        uint8_t reserved0 : 6;
        uint8_t chip_en : 1;
        uint8_t reserved1 : 1;
    };
    uint8_t value;
} DrvLp5018Config0;

typedef union {
    struct {
        uint8_t led_global_off : 1;
        uint8_t max_current_option : 1;
        uint8_t pwm_dithering : 1;
        uint8_t auto_incr_en : 1;
        uint8_t power_save_en : 1;
        uint8_t log_scale_en : 1;
        uint8_t reserved : 2;
    };
    uint8_t value;
} DrvLp5018Config1;

typedef struct {
    uint8_t device_address;

    DrvLp5018Config config;
    DrvLp5018Interface interface;
} DrvLp5018Private;

extern DrvLp5018Private drv_lp5018_private;

int DrvLp5018_WriteRegister(uint8_t address, const uint8_t* data, const uint16_t length);
int DrvLp5018_ReadRegister(uint8_t address, uint8_t* data, const uint16_t length);

#endif