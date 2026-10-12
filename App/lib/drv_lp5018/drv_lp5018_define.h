#ifndef DRV_LP5018_DEFINE_H
#define DRV_LP5018_DEFINE_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    DRV_LP5018_DEVICE_ADDRESS_0,
    DRV_LP5018_DEVICE_ADDRESS_1,
    DRV_LP5018_DEVICE_ADDRESS_2,
    DRV_LP5018_DEVICE_ADDRESS_3,
} DrvLp5018DeviceAddress;

typedef enum {
    DRV_LP5018_OUT_0,
    DRV_LP5018_OUT_1,
    DRV_LP5018_OUT_2,
    DRV_LP5018_OUT_3,
    DRV_LP5018_OUT_4,
    DRV_LP5018_OUT_5,
    DRV_LP5018_OUT_6,
    DRV_LP5018_OUT_7,
    DRV_LP5018_OUT_8,
    DRV_LP5018_OUT_9,
    DRV_LP5018_OUT_10,
    DRV_LP5018_OUT_11,
    DRV_LP5018_OUT_12,
    DRV_LP5018_OUT_13,
    DRV_LP5018_OUT_14,
    DRV_LP5018_OUT_15,
    DRV_LP5018_OUT_16,
    DRV_LP5018_OUT_17,
    DRV_LP5018_OUT_18,
    DRV_LP5018_OUT_19,
    DRV_LP5018_OUT_20,
    DRV_LP5018_OUT_21,
    DRV_LP5018_OUT_22,
    DRV_LP5018_OUT_23,
} DrvLp5018Output;

typedef enum {
    DRV_LP5018_LED_0,
    DRV_LP5018_LED_1,
    DRV_LP5018_LED_2,
    DRV_LP5018_LED_3,
    DRV_LP5018_LED_4,
    DRV_LP5018_LED_5,
    DRV_LP5018_LED_6,
    DRV_LP5018_LED_7,
} DrvLp5018Led;

typedef struct {
    int (*write)(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                 const uint16_t length);
    int (*read)(uint8_t device_address, uint8_t register_address, uint8_t* data,
                const uint16_t length);
    void (*delayMs)(uint32_t ms);
} DrvLp5018Interface;

typedef struct {
    DrvLp5018DeviceAddress device_address;
} DrvLp5018Config;

#endif