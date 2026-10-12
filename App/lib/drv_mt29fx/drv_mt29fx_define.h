#ifndef DRV_MT29FX_DEFINE_H
#define DRV_MT29FX_DEFINE_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum {
    DRV_MT29FX_COLUMN_PART_0,
    DRV_MT29FX_COLUMN_PART_1,
    DRV_MT29FX_COLUMN_PART_2,
    DRV_MT29FX_COLUMN_PART_3,
} DrvMt29FxColumnPart;

typedef struct {
    int (*write)(uint8_t* data, uint16_t length);
    int (*read)(uint8_t* data, uint16_t length);
    void (*delayMs)(const uint32_t ms);
    void (*setCsPin)(uint8_t state);
    void (*setWpPin)(uint8_t state);
    void (*setHoldPin)(uint8_t state);
} DrvMt29FxInterface;

#pragma pack(push, 1)
typedef struct {
    uint16_t column;
    uint8_t page;
    uint16_t block;
} DrvMt29FxAddress;
#pragma pack(pop)

#endif  // DRV_MT29FX_DEFINE_H
