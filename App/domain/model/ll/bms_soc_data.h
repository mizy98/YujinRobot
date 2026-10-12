#ifndef BMS_SOC_DATA_H
#define BMS_SOC_DATA_H

#include <stdint.h>

#pragma pack(push, 1)

typedef struct {
    uint16_t real_soc;     // 0.1%
    uint16_t display_soc;  // 0.1%
    uint16_t coulomb_soc;  // 0.1%

    uint32_t capacity;                // mAh
    uint32_t tick_since_calibration;  // ms

    uint16_t soh;    // 0.1%
    uint16_t cycle;  // 0.1 cycle
} BmsSocData;

#pragma pack(pop)

#endif