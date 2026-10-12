#ifndef CRC16_MODBUS_H
#define CRC16_MODBUS_H

#include <stdint.h>

uint16_t Crc16Modbus_Calculate(uint8_t* data, uint16_t length);

#endif