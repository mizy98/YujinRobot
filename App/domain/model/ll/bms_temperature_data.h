#ifndef TEMPERATURE_DATA_H
#define TEMPERATURE_DATA_H

#include <stdint.h>

#include "app_config.h"

#pragma pack(push, 1)

typedef struct {
#if TEMPERATURE_MAX_PACK_COUNT > 0
    int16_t temperatures[TEMPERATURE_MAX_PACK_COUNT];  // 0.1℃
#else
    int16_t temperatures[1];  // Dummy to avoid zero-length array
#endif
    int16_t max_temperature;      // 0.1℃
    int16_t min_temperature;      // 0.1℃
    int16_t average_temperature;  // 0.1℃
    uint16_t dt;                  // 0.1℃
    uint8_t max_temperature_number;
    uint8_t min_temperature_number;
} BmsPackTemperatureData;

typedef struct {
#if TEMPERATURE_MAX_BOARD_COUNT > 0
    int16_t temperatures[TEMPERATURE_MAX_BOARD_COUNT];  // 0.1℃
#else
    int16_t temperatures[1];  // Dummy to avoid zero-length array
#endif
    int16_t max_temperature;      // 0.1℃
    int16_t min_temperature;      // 0.1℃
    int16_t average_temperature;  // 0.1℃
    uint16_t dt;                  // 0.1℃
    uint8_t max_temperature_number;
    uint8_t min_temperature_number;
} BmsBoardTemperatureData;

typedef struct {
#if TEMPERATURE_MAX_BALANCER_COUNT > 0
    int16_t temperatures[TEMPERATURE_MAX_BALANCER_COUNT];  // 0.1℃
#else
    int16_t temperatures[1];  // Dummy to avoid zero-length array
#endif
    int16_t max_temperature;      // 0.1℃
    int16_t min_temperature;      // 0.1℃
    int16_t average_temperature;  // 0.1℃
    uint16_t dt;                  // 0.1℃
    uint8_t max_temperature_number;
    uint8_t min_temperature_number;
} BmsBalancerTemperatureData;

typedef struct {
#if TEMPERATURE_MAX_CONTACTOR_COUNT > 0
    int16_t temperatures[TEMPERATURE_MAX_CONTACTOR_COUNT];  // 0.1℃
#else
    int16_t temperatures[1];  // Dummy to avoid zero-length array
#endif
    int16_t max_temperature;      // 0.1℃
    int16_t min_temperature;      // 0.1℃
    int16_t average_temperature;  // 0.1℃
    uint16_t dt;                  // 0.1℃
    uint8_t max_temperature_number;
    uint8_t min_temperature_number;
} BmsContactorTemperatureData;

#pragma pack(pop)

#endif