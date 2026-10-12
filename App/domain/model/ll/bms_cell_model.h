#ifndef BMS_CELL_MODEL_H
#define BMS_CELL_MODEL_H

#include <stdint.h>

enum BmsCellModel {
    BMS_CELL_MODEL_UNKNOWN = 0,
    BMS_CELL_MODEL_LGES_INR21700_M50LT = 1,
    BMS_CELL_MODEL_EVE_LF105 = 2,
    BMS_CELL_MODEL_EVE_LF150 = 3,
    BMS_CELL_MODEL_SKON_E600 = 4,
};
typedef uint8_t BmsCellModel;

static inline float BmsCellModel_GetCapacity(BmsCellModel model) {
    switch (model) {
        case BMS_CELL_MODEL_LGES_INR21700_M50LT:
            return 4.8f;  // Ah
        case BMS_CELL_MODEL_EVE_LF105:
            return 100.0f;  // Ah
        case BMS_CELL_MODEL_EVE_LF150:
            return 150.0f;  // Ah
        case BMS_CELL_MODEL_SKON_E600:
            return 60.0f;  // Ah
        default:
            return 0.0f;
    }
}

#endif