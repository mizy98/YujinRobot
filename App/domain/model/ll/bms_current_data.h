#ifndef CURRENT_DATA_H
#define CURRENT_DATA_H

#include <stdint.h>

#pragma pack(push, 1)

typedef struct {
    int16_t value;                    // 0.1A
    int16_t filtered_value;           // 0.1A
    uint16_t adc_offset_voltage;      // mV
    uint16_t adc_voltage;             // mV
    uint32_t tick_since_calibration;  // ms
} BmsCurrentData;

#pragma pack(pop)

#endif