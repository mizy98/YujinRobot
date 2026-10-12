#ifndef VOLTAGE_DATA_H
#define VOLTAGE_DATA_H

#include <stdint.h>

#include "app_config.h"

#pragma pack(push, 1)
typedef struct {
    uint16_t voltage : 15;      // mV (0 - 32767 mV)
    uint16_t is_balancing : 1;  // 0: not balancing, 1: balancing
} BmsBankVoltageData;

typedef struct {
    BmsBankVoltageData bank_voltages[VOLTAGE_MAX_COUNT];  // mV
    uint16_t pack_voltage;                                // 0.1V
    uint16_t max_bank_voltage;                            // mV
    uint16_t min_bank_voltage;                            // mV
    uint16_t average_bank_voltage;                        // mV
    uint16_t dv;                                          // mV
    uint8_t max_voltage_bank_number;                      // 1 - VOLTAGE_MAX_COUNT
    uint8_t min_voltage_bank_number;                      // 1 - VOLTAGE_MAX_COUNT
} BmsVoltageData;

#pragma pack(pop)

#endif