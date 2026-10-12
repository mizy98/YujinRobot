#include "soc_lfp.h"

#define MAX_TABLE_COUNT (8u)
#define MAX_ENTRY_COUNT (26u)

#pragma pack(push, 1)
typedef struct {
    int16_t temperature;   // 0.1°C
    float capacity_ratio;  // 0.0 ~ 1.0
} CapacityRatioPoint;

typedef struct {
    float soc;  // %
    float ocv;  // V
} OcvPoint;

typedef struct {
    int16_t temperature;  // 0.1°C
    uint8_t count;
    OcvPoint table[MAX_ENTRY_COUNT];
} SocOcvTable;

static const SocOcvTable ocv_table[MAX_TABLE_COUNT] = {
    // -12°C
    {
        -120,
        11u,
        {
            {100.0f, 3.3527f},
            {89.4f, 3.2928f},
            {78.9f, 3.2890f},
            {68.3f, 3.2831f},
            {57.7f, 3.2769f},
            {47.2f, 3.2710f},
            {36.6f, 3.2658f},
            {26.0f, 3.2642f},
            {15.5f, 3.2556f},
            {4.9f, 3.2512f},
            {0.0f, 3.2489f},
        },
    },

    // -5°C
    {
        -50,
        14u,
        {
            {100.0f, 3.3216f},
            {91.9f, 3.2983f},
            {83.8f, 3.2971f},
            {75.8f, 3.2949f},
            {67.7f, 3.2908f},
            {59.6f, 3.2854f},
            {51.5f, 3.2795f},
            {43.4f, 3.2737f},
            {35.4f, 3.2686f},
            {27.3f, 3.2641f},
            {19.2f, 3.2603f},
            {11.1f, 3.2567f},
            {3.0f, 3.2529f},
            {0.0f, 3.2485f},
        },
    },

    // 0°C
    {
        0,
        19u,
        {
            {100.0f, 3.4747f}, {94.4f, 3.3310f}, {88.7f, 3.3254f}, {83.1f, 3.3194f},
            {77.5f, 3.3127f},  {71.9f, 3.3045f}, {66.2f, 3.2961f}, {60.6f, 3.2883f},
            {55.0f, 3.2819f},  {49.3f, 3.2763f}, {43.7f, 3.2720f}, {38.1f, 3.2673f},
            {32.4f, 3.2614f},  {26.8f, 3.2541f}, {21.2f, 3.2440f}, {15.6f, 3.2302f},
            {9.9f, 3.2099f},   {4.3f, 3.1814f},  {0.0f, 3.1800f},
        },
    },

    // 5°C
    {
        50,
        19u,
        {
            {100.0f, 3.3577f}, {94.2f, 3.3075f}, {88.5f, 3.3074f}, {82.7f, 3.3065f},
            {76.9f, 3.3032f},  {71.1f, 3.2977f}, {65.4f, 3.2911f}, {59.6f, 3.2844f},
            {53.8f, 3.2784f},  {48.1f, 3.2739f}, {42.3f, 3.2703f}, {36.5f, 3.2667f},
            {30.7f, 3.2629f},  {25.0f, 3.2586f}, {19.2f, 3.2532f}, {13.4f, 3.2458f},
            {7.7f, 3.2353f},   {1.9f, 3.2191f},  {0.0f, 3.2055f},
        },
    },

    // 10°C
    {
        100,
        21u,
        {
            {100.0f, 3.4112f}, {94.9f, 3.3126f}, {89.8f, 3.3134f}, {84.7f, 3.3134f},
            {79.6f, 3.3146f},  {74.5f, 3.3128f}, {69.4f, 3.3068f}, {64.3f, 3.2972f},
            {59.1f, 3.2882f},  {54.0f, 3.2813f}, {48.9f, 3.2794f}, {43.8f, 3.2752f},
            {38.7f, 3.2733f},  {33.6f, 3.2709f}, {28.5f, 3.2670f}, {23.4f, 3.2608f},
            {18.3f, 3.2512f},  {13.2f, 3.2355f}, {8.1f, 3.2123f},  {2.9f, 3.1889f},
            {0.0f, 3.1800f},
        },
    },

    // 13°C
    {
        130,
        21u,
        {
            {100.0f, 3.4022f}, {94.9f, 3.3148f}, {89.8f, 3.3163f}, {84.7f, 3.3175f},
            {79.6f, 3.3173f},  {74.5f, 3.3157f}, {69.4f, 3.3104f}, {64.3f, 3.3001f},
            {59.2f, 3.2901f},  {54.1f, 3.2832f}, {49.0f, 3.2794f}, {43.9f, 3.2775f},
            {38.8f, 3.2759f},  {33.7f, 3.2737f}, {28.6f, 3.2695f}, {23.5f, 3.2625f},
            {18.4f, 3.2515f},  {13.3f, 3.2345f}, {8.2f, 3.2105f},  {3.1f, 3.1863f},
            {0.0f, 3.1607f},
        },
    },

    // 29°C
    {
        290,
        22u,
        {
            {100.0f, 3.4728f}, {95.1f, 3.3241f}, {90.2f, 3.3242f}, {85.3f, 3.3240f},
            {80.4f, 3.3239f},  {75.5f, 3.3236f}, {70.6f, 3.3230f}, {65.6f, 3.3204f},
            {60.7f, 3.3066f},  {55.8f, 3.2915f}, {50.9f, 3.2866f}, {46.0f, 3.2847f},
            {41.1f, 3.2835f},  {36.2f, 3.2820f}, {31.3f, 3.2788f}, {26.4f, 3.2711f},
            {21.5f, 3.2559f},  {16.6f, 3.2407f}, {11.7f, 3.2160f}, {6.7f, 3.1947f},
            {1.8f, 3.1677f},   {0.0f, 3.1677f},
        },
    },

    // 43°C
    {
        430,
        22u,
        {
            {100.0f, 3.4153f}, {95.3f, 3.3287f}, {90.5f, 3.3283f}, {85.8f, 3.3276f},
            {81.0f, 3.3271f},  {76.3f, 3.3269f}, {71.5f, 3.3268f}, {66.8f, 3.3260f},
            {62.0f, 3.3186f},  {57.3f, 3.2979f}, {52.5f, 3.2917f}, {47.8f, 3.2917f},
            {43.0f, 3.2907f},  {38.3f, 3.2897f}, {33.5f, 3.2879f}, {28.8f, 3.2747f},
            {24.0f, 3.2583f},  {19.3f, 3.2437f}, {14.5f, 3.2202f}, {9.8f, 3.2001f},
            {5.0f, 3.1773f},   {0.0f, 3.1773f},
        },
    },
};

static const CapacityRatioPoint capacity_ratio_table[] = {
    {-120, 0.49f}, {-50, 0.65f}, {0, 0.85f}, {50, 0.91f}, {100, 1.00f},
};

#pragma pack(pop)

/* ── 단일 테이블에서 OCV → SOC 선형 보간 ──────────────────
   주의: OCV는 SOC에 대해 단조감소(monotone decreasing)이므로
         테이블을 순방향 탐색해서 OCV가 구간 안에 들어오면 역보간.
         단, 고SOC 영역(~100% 근처)은 OCV가 일시적으로 높아지는
         피크가 있을 수 있으므로, 피크 이후 구간부터 탐색 시작.
   반환: SOC [%], 범위 벗어나면 클램프 값 반환                */
static float _lookupSocFromOcv(const SocOcvTable* ocv_table, float ocv) {
    uint8_t i;
    float ocv_hi, ocv_lo, soc_hi, soc_lo, ratio;

    // clamp temperature over the range of the table
    if (ocv >= ocv_table->table[0u].ocv) {
        return ocv_table->table[0u].soc;
    }

    // clamp temperature under the range of the table
    if (ocv <= ocv_table->table[ocv_table->count - 1u].ocv) {
        return ocv_table->table[ocv_table->count - 1u].soc;
    }

    // search for the section where OCV belongs (OCV is monotone decreasing)
    for (i = 0u; i < (ocv_table->count - 1u); i++) {
        ocv_hi = ocv_table->table[i].ocv;
        ocv_lo = ocv_table->table[i + 1u].ocv;

        if ((ocv <= ocv_hi) && (ocv >= ocv_lo)) {
            soc_hi = ocv_table->table[i].soc;
            soc_lo = ocv_table->table[i + 1u].soc;
            ratio = (ocv - ocv_lo) / (ocv_hi - ocv_lo);
            return soc_lo + (soc_hi - soc_lo) * ratio;
        }
    }

    return ocv_table->table[ocv_table->count - 1u].soc;
}

float _lookupSoc(int16_t temperature, float ocv) {
    uint8_t i;
    float soc_lo, soc_hi, ratio;

    // clamp temperature under the range of the table, and extrapolate if out of range
    if (temperature <= (float)ocv_table[0u].temperature) {
        return _lookupSocFromOcv(&ocv_table[0u], ocv);
    }

    // clamp temperature over the range of the table, and extrapolate if out of range
    if (temperature >= (float)ocv_table[MAX_TABLE_COUNT - 1u].temperature) {
        return _lookupSocFromOcv(&ocv_table[MAX_TABLE_COUNT - 1u], ocv);
    }

    // search the temperature section, and extrapolate if out of range
    for (i = 0u; i < (MAX_TABLE_COUNT - 1u); i++) {
        if (temperature < (float)ocv_table[i + 1u].temperature) {
            break;
        }
    }

    soc_lo = _lookupSocFromOcv(&ocv_table[i], ocv);
    soc_hi = _lookupSocFromOcv(&ocv_table[i + 1u], ocv);

    ratio = (temperature - (float)ocv_table[i].temperature) /
            (float)(ocv_table[i + 1u].temperature - ocv_table[i].temperature);

    return soc_lo + (soc_hi - soc_lo) * ratio;
}

float _lookupCapacityRatio(int16_t temperature) {
    uint8_t i;
    float ratio_lo, ratio_hi, ratio;

    // clamp temperature under the range of the table, and extrapolate if out of range
    if (temperature <= capacity_ratio_table[0u].temperature) {
        return capacity_ratio_table[0u].capacity_ratio;
    }

    // clamp temperature over the range of the table, and extrapolate if out of range
    if (temperature >=
        capacity_ratio_table[sizeof(capacity_ratio_table) / sizeof(CapacityRatioPoint) - 1u]
            .temperature) {
        return capacity_ratio_table[sizeof(capacity_ratio_table) / sizeof(CapacityRatioPoint) - 1u]
            .capacity_ratio;
    }

    // search the temperature section, and extrapolate if out of range
    for (i = 0u; i < (sizeof(capacity_ratio_table) / sizeof(CapacityRatioPoint) - 1u); i++) {
        if (temperature < capacity_ratio_table[i + 1u].temperature) {
            break;
        }
    }

    ratio_lo = capacity_ratio_table[i].capacity_ratio;
    ratio_hi = capacity_ratio_table[i + 1u].capacity_ratio;

    ratio = (temperature - (float)capacity_ratio_table[i].temperature) /
            (float)(capacity_ratio_table[i + 1u].temperature - capacity_ratio_table[i].temperature);

    return ratio_lo + (ratio_hi - ratio_lo) * ratio;
}

float SocLfp_CalculateSoc(int16_t temperature, uint16_t ocv) {
    float soc = _lookupSoc(temperature, (float)ocv / 1000.0f);
    if (soc < 0.0f) {
        soc = 0.0f;
    } else if (soc > 100.0f) {
        soc = 100.0f;
    }

    return soc;
}

float SocLfp_CalculateCapacityRatioByTemperature(int16_t temperature) {
    float ratio = _lookupCapacityRatio(temperature);
    if (ratio < 0.0f) {
        ratio = 0.0f;
    } else if (ratio > 1.0f) {
        ratio = 1.0f;
    }

    return ratio;
}

float SocLfp_CalculateCapacityByCycle(float cycle) {
    (void)cycle;

    return 1.0f;
}

bool SocLfp_IsCalibrationRegion(int16_t temperature, uint16_t ocv) {
    (void)temperature;
    if (ocv <= 3300) {
        return true;
    }

    return false;
}

bool SocLfp_IsFullChargeRegion(int16_t temperature, uint16_t voltage) {
    (void)temperature;

    if (voltage >= 3375) {
        return true;
    }

    return false;
}
