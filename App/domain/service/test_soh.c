#include "test_soh.h"

static const TestSohCyclePoint _default_ncm_cycle_table[] = {
    {0.0f, 100.0f},   {300.0f, 96.0f},  {600.0f, 92.0f},  {900.0f, 88.0f},
    {1200.0f, 84.0f}, {1500.0f, 80.0f}, {1800.0f, 76.0f},
};

static float _clampf(float value, float min_value, float max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static float _lerp(float x0, float y0, float x1, float y1, float x) {
    float dx = x1 - x0;
    if (dx == 0.0f) {
        return y0;
    }
    return y0 + ((x - x0) * (y1 - y0) / dx);
}

float TestSohNcm_FromCapacity(float measured_capacity_ah, float nominal_capacity_ah) {
    if (nominal_capacity_ah <= 0.0f) {
        return 0.0f;
    }

    return _clampf((measured_capacity_ah / nominal_capacity_ah) * 100.0f, 0.0f, 100.0f);
}

float TestSohNcm_FromCycleLinear(float cycle_count, float cycle_fade_percent_per_cycle,
                                 float min_soh) {
    float soh;

    if (cycle_fade_percent_per_cycle < 0.0f) {
        cycle_fade_percent_per_cycle = 0.0f;
    }

    if (cycle_count < 0.0f) {
        cycle_count = 0.0f;
    }

    soh = 100.0f - (cycle_count * cycle_fade_percent_per_cycle);
    return _clampf(soh, _clampf(min_soh, 0.0f, 100.0f), 100.0f);
}

float TestSohNcm_FromResistance(float current_resistance_mohm, float initial_resistance_mohm) {
    if (initial_resistance_mohm <= 0.0f || current_resistance_mohm <= 0.0f) {
        return 0.0f;
    }

    // Resistance growth is inversely proportional to remaining SOH.
    return _clampf((initial_resistance_mohm / current_resistance_mohm) * 100.0f, 0.0f, 100.0f);
}

float TestSohNcm_FromCycleTableCustom(float cycle_count, const TestSohCyclePoint* table,
                                      uint16_t table_count) {
    uint16_t i;

    if (table == 0 || table_count == 0u) {
        return 0.0f;
    }

    if (cycle_count <= table[0u].cycle_count) {
        return _clampf(table[0u].soh, 0.0f, 100.0f);
    }

    if (cycle_count >= table[table_count - 1u].cycle_count) {
        return _clampf(table[table_count - 1u].soh, 0.0f, 100.0f);
    }

    for (i = 0u; i < (table_count - 1u); i++) {
        float x0 = table[i].cycle_count;
        float x1 = table[i + 1u].cycle_count;

        if (cycle_count >= x0 && cycle_count <= x1) {
            float y = _lerp(x0, table[i].soh, x1, table[i + 1u].soh, cycle_count);
            return _clampf(y, 0.0f, 100.0f);
        }
    }

    return _clampf(table[table_count - 1u].soh, 0.0f, 100.0f);
}

float TestSohNcm_FromCycleTable(float cycle_count) {
    return TestSohNcm_FromCycleTableCustom(
        cycle_count, _default_ncm_cycle_table,
        (uint16_t)(sizeof(_default_ncm_cycle_table) / sizeof(_default_ncm_cycle_table[0])));
}

float TestSohNcm_Calculate(TestSohMethod method, const TestSohNcmInput* input,
                           const TestSohNcmConfig* config) {
    float soh_capacity;
    float soh_cycle;
    float soh_resistance;
    float w_sum;

    if (input == 0) {
        return 0.0f;
    }

    switch (method) {
        case TEST_SOH_METHOD_CAPACITY_RATIO:
            return TestSohNcm_FromCapacity(input->measured_capacity_ah, input->nominal_capacity_ah);

        case TEST_SOH_METHOD_CYCLE_LINEAR:
            if (config == 0) {
                return 0.0f;
            }
            return TestSohNcm_FromCycleLinear(
                input->cycle_count, config->cycle_fade_percent_per_cycle, config->min_soh);

        case TEST_SOH_METHOD_RESISTANCE_RATIO:
            return TestSohNcm_FromResistance(input->current_resistance_mohm,
                                             input->initial_resistance_mohm);

        case TEST_SOH_METHOD_CYCLE_TABLE:
            if (config != 0 && config->cycle_table != 0 && config->cycle_table_count > 0u) {
                return TestSohNcm_FromCycleTableCustom(input->cycle_count, config->cycle_table,
                                                       config->cycle_table_count);
            }
            return TestSohNcm_FromCycleTable(input->cycle_count);

        case TEST_SOH_METHOD_HYBRID_WEIGHTED:
            if (config == 0) {
                return 0.0f;
            }

            soh_capacity =
                TestSohNcm_FromCapacity(input->measured_capacity_ah, input->nominal_capacity_ah);

            if (config->cycle_table != 0 && config->cycle_table_count > 0u) {
                soh_cycle = TestSohNcm_FromCycleTableCustom(input->cycle_count, config->cycle_table,
                                                            config->cycle_table_count);
            } else {
                soh_cycle = TestSohNcm_FromCycleTable(input->cycle_count);
            }

            soh_resistance = TestSohNcm_FromResistance(input->current_resistance_mohm,
                                                       input->initial_resistance_mohm);

            w_sum = config->hybrid_weight_capacity + config->hybrid_weight_cycle +
                    config->hybrid_weight_resistance;
            if (w_sum <= 0.0f) {
                return 0.0f;
            }

            return _clampf((soh_capacity * config->hybrid_weight_capacity +
                            soh_cycle * config->hybrid_weight_cycle +
                            soh_resistance * config->hybrid_weight_resistance) /
                               w_sum,
                           0.0f, 100.0f);

        default:
            return 0.0f;
    }
}
