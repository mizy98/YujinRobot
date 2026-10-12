#ifndef TEST_SOH_H
#define TEST_SOH_H

#include <stdint.h>

typedef enum {
    TEST_SOH_METHOD_CAPACITY_RATIO = 0,
    TEST_SOH_METHOD_CYCLE_LINEAR,
    TEST_SOH_METHOD_RESISTANCE_RATIO,
    TEST_SOH_METHOD_CYCLE_TABLE,
    TEST_SOH_METHOD_HYBRID_WEIGHTED,
} TestSohMethod;

typedef struct {
    float cycle_count;
    float soh;
} TestSohCyclePoint;

typedef struct {
    float measured_capacity_ah;
    float nominal_capacity_ah;
    float cycle_count;
    float current_resistance_mohm;
    float initial_resistance_mohm;
} TestSohNcmInput;

typedef struct {
    float cycle_fade_percent_per_cycle;
    float min_soh;

    float hybrid_weight_capacity;
    float hybrid_weight_cycle;
    float hybrid_weight_resistance;

    const TestSohCyclePoint* cycle_table;
    uint16_t cycle_table_count;
} TestSohNcmConfig;

float TestSohNcm_FromCapacity(float measured_capacity_ah, float nominal_capacity_ah);
float TestSohNcm_FromCycleLinear(float cycle_count, float cycle_fade_percent_per_cycle,
                                 float min_soh);
float TestSohNcm_FromResistance(float current_resistance_mohm, float initial_resistance_mohm);
float TestSohNcm_FromCycleTable(float cycle_count);
float TestSohNcm_FromCycleTableCustom(float cycle_count, const TestSohCyclePoint* table,
                                      uint16_t table_count);

float TestSohNcm_Calculate(TestSohMethod method, const TestSohNcmInput* input,
                           const TestSohNcmConfig* config);

#endif
