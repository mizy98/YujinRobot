#ifndef BMS_PROTECTION_CONFIG_H
#define BMS_PROTECTION_CONFIG_H

#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint8_t code;
    uint8_t is_enabled : 1;
    uint8_t is_release_enabled : 1;
    uint8_t is_pf_enabled : 1;
    uint8_t _ : 5;
    int32_t threshold_value;          // voltage: mV, temperature: 0.1℃, current: 0.1A
    uint32_t threshold_tick;          // ms
    int32_t release_threshold_value;  // voltage: mV, temperature: 0.1℃, current: 0.1A
    uint32_t release_threshold_tick;  // ms
} BmsProtectionConfig;

#pragma pack(pop)

#endif