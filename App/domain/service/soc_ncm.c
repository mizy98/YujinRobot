#include "soc_ncm.h"

typedef struct {
    float soc;  // %
    float ocv;  // V
} OcvPoint;

static const OcvPoint m50lt_ocv_table[] = {
    {0.0f, 3.200}, {0.3, 3.240},  {0.7, 3.273},  {2.3, 3.318},  {4.6, 3.373},  {7.6, 3.398},
    {10.6, 3.419}, {14.1, 3.451}, {16.5, 3.470}, {19.4, 3.493}, {22.5, 3.519}, {25.3, 3.537},
    {28.4, 3.557}, {31.2, 3.572}, {34.4, 3.590}, {37.1, 3.603}, {40.4, 3.618}, {43.0, 3.630},
    {46.3, 3.644}, {48.9, 3.657}, {52.3, 3.675}, {55.4, 3.691}, {58.2, 3.706}, {61.2, 3.724},
    {64.2, 3.741}, {67.1, 3.764}, {70.2, 3.788}, {73.0, 3.808}, {76.1, 3.830}, {82.1, 3.874},
    {84.8, 3.897}, {88.0, 3.911}, {91.0, 3.924}, {94.0, 3.944}, {97.2, 3.963}, {100.0, 3.987},
};

float SocNcm_CalculateSoc(int16_t temperature, uint16_t ocv) {
    float converted_ocv = ocv / 1000.0f;

    const int table_size = sizeof(m50lt_ocv_table) / sizeof(m50lt_ocv_table[0]);

    if (converted_ocv <= m50lt_ocv_table[0].ocv) {
        return m50lt_ocv_table[0].soc;
    }
    if (converted_ocv >= m50lt_ocv_table[table_size - 1].ocv) {
        return m50lt_ocv_table[table_size - 1].soc;
    }

    for (int i = 0; i < table_size - 1; i++) {
        if (converted_ocv >= m50lt_ocv_table[i].ocv &&
            converted_ocv <= m50lt_ocv_table[i + 1].ocv) {
            float ocv_diff = m50lt_ocv_table[i + 1].ocv - m50lt_ocv_table[i].ocv;
            float soc_diff = m50lt_ocv_table[i + 1].soc - m50lt_ocv_table[i].soc;

            if (ocv_diff == 0.0f) {
                return m50lt_ocv_table[i].soc;
            }

            float ratio = (converted_ocv - m50lt_ocv_table[i].ocv) / ocv_diff;
            float interpolated_soc = m50lt_ocv_table[i].soc + (soc_diff * ratio);

            if (interpolated_soc < 0.0f) return 0.0f;
            if (interpolated_soc > 100.0f) return 100.0f;

            return interpolated_soc;
        }
    }

    // does not reach here
    return 0.0f;
}

float SocNcm_CalculateCapacityRatioByTemperature(int16_t temperature) {
    // TODO: Implement the actual capacity ratio calculation based on the temperature for NCM cells.
    (void)temperature;
    return 0.75f;
}

float SocNcm_CalculateCapacityByCycle(float cycle) {
    // TODO: Implement the actual capacity calculation based on the cycle for NCM cells.
    (void)cycle;
    return 1.0f;
}