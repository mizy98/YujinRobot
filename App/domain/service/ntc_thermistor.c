#include "ntc_thermistor.h"

#define self (ntc_thermistor_private)

static struct {
    float serial_resistor;  // kΩ
    uint16_t adc_resolution;
} ntc_thermistor_private;

typedef struct {
    float temperature_celsius;
    float resistor_k_ohm;
    uint16_t adc_value;
} NtcEntry;

static NtcEntry ntc_10k_3950_table[] = {
    {-30, 177.000f},  //
    {-29, 166.400f}, {-28, 156.600f}, {-27, 147.200f}, {-26, 138.500f}, {-25, 133.500f},
    {-24, 125.672f}, {-23, 118.350f}, {-22, 111.498f}, {-21, 105.030f}, {-20, 99.050f},
    {-19, 93.447f},  {-18, 88.175f},  {-17, 83.227f},  {-16, 78.591f},  {-15, 74.238f},
    {-14, 70.153f},  {-13, 66.316f},  {-12, 62.712f},  {-11, 59.325f},  {-10, 56.142f},
    {-9, 53.148f},   {-8, 50.331f},   {-7, 47.680f},   {-6, 45.184f},   {-5, 42.834f},
    {-4, 40.612f},   {-3, 38.533f},   {-2, 36.566f},   {-1, 34.710f},   {0, 32.960f},
    {1, 31.308f},    {2, 29.749f},    {3, 28.276f},    {4, 26.885f},    {5, 25.570f},
    {6, 24.327f},    {7, 23.152f},    {8, 22.041f},    {9, 20.989f},    {10, 19.993f},
    {11, 19.051f},   {12, 18.158f},   {13, 17.312f},   {14, 16.512f},   {15, 15.751f},
    {16, 15.031f},   {17, 14.347f},   {18, 13.699f},   {19, 13.083f},   {20, 12.499f},
    {21, 11.944f},   {22, 11.417f},   {23, 10.916f},   {24, 10.440f},   {25, 10.000f},
    {26, 9.557f},    {27, 9.147f},    {28, 8.758f},    {29, 8.387f},    {30, 8.034f},
    {31, 7.698f},    {32, 7.377f},    {33, 7.072f},    {34, 6.781f},    {35, 6.503f},
    {36, 6.239f},    {37, 5.987f},    {38, 5.746f},    {39, 5.517f},    {40, 5.298f},
    {41, 5.088f},    {42, 4.888f},    {43, 4.697f},    {44, 4.515f},    {45, 4.341f},
    {46, 4.174f},    {47, 4.015f},    {48, 3.862f},    {49, 3.716f},    {50, 3.588f},
    {51, 3.443f},    {52, 3.315f},    {53, 3.183f},    {54, 3.076f},    {55, 2.963f},
    {56, 2.856f},    {57, 2.753f},    {58, 2.654f},    {59, 2.559f},    {60, 2.468f},
    {61, 2.381f},    {62, 2.297f},    {63, 2.217f},    {64, 2.140f},    {65, 2.066f},
    {66, 1.995f},    {67, 1.927f},    {68, 1.861f},    {69, 1.798f},    {70, 1.738f},
    {71, 1.679f},    {72, 1.624f},    {73, 1.570f},    {74, 1.518f},    {75, 1.473f},
    {76, 1.420f},    {77, 1.374f},    {78, 1.330f},    {79, 1.287f},    {80, 1.246f},
    {81, 1.207f},    {82, 1.169f},    {83, 1.132f},    {84, 1.097f},    {85, 1.062f},
    {86, 1.030f},    {87, 0.998f},    {88, 0.967f},    {89, 0.938f},    {90, 0.910f},
    {91, 0.882f},    {92, 0.856f},    {93, 0.830f},    {94, 0.805f},    {95, 0.782f},
    {96, 0.759f},    {97, 0.737f},    {98, 0.715f},    {99, 0.694f},    {100, 0.674f},
    {101, 0.655f},   {102, 0.636f},   {103, 0.618f},   {104, 0.601f},   {105, 0.584f},
    {106, 0.568f},   {107, 0.552f},   {108, 0.537f},   {109, 0.522f},   {110, 0.508f},
    {111, 0.494f},   {112, 0.480f},   {113, 0.467f},   {114, 0.455f},   {115, 0.442f},
    {116, 0.431f},   {117, 0.419f},   {118, 0.408f},   {119, 0.398f},   {120, 0.387f},
    {121, 0.377f},   {122, 0.367f},   {123, 0.358f},   {124, 0.349f},   {125, 0.340f}};

static NtcEntry ntc_10k_3630_table[] = {
    {-25, 99.310f}, {-24, 0},     {-23, 0},      {-22, 0},       {-21, 0},       {-20, 76.550f},
    {-19, 0},       {-18, 0},     {-17, 0},      {-16, 0},       {-15, 59.450f}, {-14, 0},
    {-13, 0},       {-12, 0},     {-11, 0},      {-10, 46.510f}, {-9, 0},        {-8, 0},
    {-7, 0},        {-6, 0},      {-5, 36.630f}, {-4, 0},        {-3, 0},        {-2, 0},
    {-1, 0},        {0, 29.050f}, {1, 0},        {2, 0},         {3, 0},         {4, 0},
    {5, 23.190f},   {6, 0},       {7, 0},        {8, 0},         {9, 0},         {10, 18.620f},
    {11, 0},        {12, 0},      {13, 0},       {14, 0},        {15, 15.050f},  {16, 0},
    {17, 0},        {18, 0},      {19, 0},       {20, 12.230f},  {21, 0},        {22, 0},
    {23, 0},        {24, 0},      {25, 10.000f}, {26, 0},        {27, 0},        {28, 0},
    {29, 0},        {30, 8.219f}, {31, 0},       {32, 0},        {33, 0},        {34, 0},
    {35, 6.792f},   {36, 0},      {37, 0},       {38, 0},        {39, 0},        {40, 5.641f},
    {41, 0},        {42, 0},      {43, 0},       {44, 0},        {45, 4.708f},   {46, 0},
    {47, 0},        {48, 0},      {49, 0},       {50, 3.949f},   {51, 0},        {52, 0},
    {53, 0},        {54, 0},      {55, 3.327f},  {56, 0},        {57, 0},        {58, 0},
    {59, 0},        {60, 2.816f}, {61, 0},       {62, 0},        {63, 0},        {64, 0},
    {65, 2.393f},   {66, 0},      {67, 0},       {68, 0},        {69, 0},        {70, 2.043f},
    {71, 0},        {72, 0},      {73, 0},       {74, 0},        {75, 1.751f},   {76, 0},
    {77, 0},        {78, 0},      {79, 0},       {80, 1.506f},   {81, 0},        {82, 0},
    {83, 0},        {84, 0},      {85, 1.301f},  {86, 0},        {87, 0},        {88, 0},
    {89, 0},        {90, 1.128f}, {91, 0},       {92, 0},        {93, 0},        {94, 0},
    {95, 0.981f},   {96, 0},      {97, 0},       {98, 0},        {99, 0},        {100, 0.856f},
    {101, 0},       {102, 0},     {103, 0},      {104, 0},       {105, 0.750f},  {106, 0},
    {107, 0},       {108, 0},     {109, 0},      {110, 0.659f},  {111, 0},       {112, 0},
    {113, 0},       {114, 0},     {115, 0.581f}, {116, 0},       {117, 0},       {118, 0},
    {119, 0},       {120, 0.514f}};

static void _initializeTable() {
    for (uint16_t i = 0; i < ARRAY_COUNT(ntc_10k_3950_table); ++i) {
        ntc_10k_3950_table[i].adc_value =
            (uint16_t)((ntc_10k_3950_table[i].resistor_k_ohm /
                        (ntc_10k_3950_table[i].resistor_k_ohm + self.serial_resistor)) *
                       self.adc_resolution);
    }

    uint16_t table_size = ARRAY_COUNT(ntc_10k_3630_table);
    uint16_t loop = table_size / 5;

    for (uint16_t i = 0; i < loop; ++i) {
        if ((i * 5 + 5) >= table_size) break;

        float step = (ntc_10k_3630_table[i * 5].resistor_k_ohm -
                      ntc_10k_3630_table[i * 5 + 5].resistor_k_ohm) /
                     5.0f;

        for (uint8_t j = 1; j < 5; ++j) {
            uint16_t index = (i * 5) + j;
            if (index >= table_size) break;

            ntc_10k_3630_table[index].resistor_k_ohm =
                ntc_10k_3630_table[i * 5].resistor_k_ohm - (step * j);
        }
    }

    for (uint16_t i = 0; i < table_size; ++i) {
        ntc_10k_3630_table[i].adc_value =
            (uint16_t)((ntc_10k_3630_table[i].resistor_k_ohm /
                        (ntc_10k_3630_table[i].resistor_k_ohm + self.serial_resistor)) *
                       self.adc_resolution);
    }
}

static float _ToTemperature(NtcEntry* table, uint16_t table_count, uint16_t adc_value) {
    // NTC 특성: 온도 상승 → 저항 감소 → ADC 값 감소
    // 테이블은 온도 오름차순으로 정렬되어 있음

    // 범위 체크: ADC 값이 최고 온도(최소 ADC)보다 작으면
    if (adc_value < table[table_count - 1].adc_value) {
        return table[table_count - 1].temperature_celsius;
    }

    // 범위 체크: ADC 값이 최저 온도(최대 ADC)보다 크면
    if (adc_value > table[0].adc_value) {
        return table[0].temperature_celsius;
    }

    // 테이블 검색 (온도 증가 순서)
    for (uint16_t i = 1; i < table_count; ++i) {
        // adc_value가 현재 항목의 ADC 값보다 크거나 같으면
        // (즉, 이전 항목과 현재 항목 사이에 위치)
        if (adc_value >= table[i].adc_value) {
            // 선형 보간
            float t1 = table[i - 1].temperature_celsius;
            float t2 = table[i].temperature_celsius;
            uint16_t v1 = table[i - 1].adc_value;
            uint16_t v2 = table[i].adc_value;

            // v1 > v2이므로 (v1 - v2)를 사용
            return t1 + (t2 - t1) * (float)(v1 - adc_value) / (float)(v1 - v2);
        }
    }

    // 여기 도달하면 안되지만 안전장치
    return table[table_count - 1].temperature_celsius;
}

void NtcThermistor_Initialize(float serial_resistor, uint32_t adc_resolution) {
    self.serial_resistor = serial_resistor;
    self.adc_resolution = adc_resolution;

    _initializeTable();
}

float NtcThermistor_GetTemperature(NtcThermistorType type, uint16_t adc) {
    switch (type) {
        case NTC_THERMISTOR_TYPE_10K_3950:
            return _ToTemperature(ntc_10k_3950_table, ARRAY_COUNT(ntc_10k_3950_table), adc);
        case NTC_THERMISTOR_TYPE_10K_3630:
            return _ToTemperature(ntc_10k_3630_table, ARRAY_COUNT(ntc_10k_3630_table), adc);
        default:
            return -25.0f;
    }
}