#ifndef BMS_EXPANSION_DATA_H
#define BMS_EXPANSION_DATA_H

#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint32_t input_voltage;             // mV
    uint32_t charge_remaining_time;     // ms
    uint32_t discharge_remaining_time;  // ms
    uint32_t remain_capacity;           // mAh
    uint32_t remain_energy;             // mWh
} BmsExpansionData;
#pragma pack(pop)

#endif