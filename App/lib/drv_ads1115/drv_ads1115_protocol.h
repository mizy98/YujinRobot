#ifndef DRV_ADS1115_PROTOCOL_H
#define DRV_ADS1115_PROTOCOL_H

#include "drv_ads1115_define.h"

#define I2C_ADDRESS 0b1001000

#define ADDRESS_POINTER 0x00
#define CONVERSION 0x00
#define CONFIG 0x01
#define LO_THRESH 0x02
#define HI_THRESH 0x03
#define RESET 0x06

#pragma pack(push, 1)
typedef struct {
    union {
        struct {
            uint8_t comp_que : 2;
            uint8_t comp_lat : 1;
            uint8_t comp_pol : 1;
            uint8_t comp_mode : 1;
            uint8_t dr : 3;
            uint8_t mode : 1;
            uint8_t pga : 3;
            uint8_t mux : 3;
            uint8_t os : 1;
        };
        uint16_t value;
    };
} ConfigRegister;
#pragma pack(pop)

void DrvAds1115_RegisterInterface(DrvAds1115Interface *interface);
int DrvAds1115_WriteRegister(uint8_t address, uint8_t *data, uint16_t data_length);
int DrvAds1115_ReadRegister(uint8_t address, uint8_t *data, uint16_t data_length);
int DrvAds1115_WriteConfig(const ConfigRegister *config);
int DrvAds1115_ReadConfig(ConfigRegister *config);

#endif  // DRV_ADS1115_PROTOCOL_H