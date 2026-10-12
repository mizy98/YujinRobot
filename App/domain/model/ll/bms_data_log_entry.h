#ifndef BMS_DATA_LOG_ENTRY_H
#define BMS_DATA_LOG_ENTRY_H

#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint32_t day_id;  // (YYYYMMDD)
    uint32_t sequence;
    uint32_t record_count;
    struct {
        uint16_t block;
        uint16_t page;
    } start_address;

    struct {
        uint16_t block;
        uint16_t page;
    } end_address;
} BmsDataLogEntry;
#pragma pack(pop)

#endif
