#ifndef BMS_SOH_DATA_H
#define BMS_SOH_DATA_H

#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t soh;  // 0.1%
} BmsSohData;

#pragma pack(pop)

#endif