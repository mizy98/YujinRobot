#include "drv_ads1115_protocol.h"

static DrvAds1115Interface ads1115_interface;

void DrvAds1115_RegisterInterface(DrvAds1115Interface *interface) {
    ads1115_interface = *interface;
}

int DrvAds1115_WriteRegister(uint8_t address, uint8_t *data, uint16_t data_length) {
    uint8_t buffer[1 + data_length];

    buffer[0] = address;

    if (data_length > 0) {
        memcpy(&buffer[1], data, data_length);
    }

    return ads1115_interface.write(I2C_ADDRESS, buffer, 1 + data_length);
}

int DrvAds1115_ReadRegister(uint8_t address, uint8_t *data, uint16_t data_length) {
    int result = ads1115_interface.write(I2C_ADDRESS, &address, 1);
    if (result != 0) {
        return result;
    }

    return ads1115_interface.read(I2C_ADDRESS, data, data_length);
}

int DrvAds1115_WriteConfig(const ConfigRegister *config) {
    uint8_t buffer[2];

    buffer[0] = (config->os << 7) | (config->mux << 4) | (config->pga << 1) | (config->mode << 0);
    buffer[1] = (config->dr << 5) | (config->comp_mode << 4) | (config->comp_pol << 3) |
                (config->comp_lat << 2) | (config->comp_que << 0);

    return DrvAds1115_WriteRegister(CONFIG, buffer, 2);
}

int DrvAds1115_ReadConfig(ConfigRegister *config) {
    uint8_t buffer[2];

    int result = DrvAds1115_ReadRegister(CONFIG, buffer, sizeof(buffer));
    if (result != 0) {
        return result;
    }

    config->os = (buffer[0] & 0b10000000) >> 7;
    config->mux = (buffer[0] & 0b01110000) >> 4;
    config->pga = (buffer[0] & 0b00001110) >> 1;
    config->mode = (buffer[0] & 0b00000001) >> 0;

    config->dr = (buffer[1] & 0b11100000) >> 5;
    config->comp_mode = (buffer[1] & 0b00010000) >> 4;
    config->comp_pol = (buffer[1] & 0b00001000) >> 3;
    config->comp_lat = (buffer[1] & 0b00000100) >> 2;
    config->comp_que = (buffer[1] & 0b00000011) >> 0;

    return result;
}