#ifndef BMS_DATA_LOG_INFO_H
#define BMS_DATA_LOG_INFO_H

#include <stdint.h>

#define BMS_DATA_LOG_INFO_SIZE (64u)

typedef struct {
    uint16_t record_unit_size;
    uint16_t page_size;
    uint16_t pages_per_block;
    uint16_t blocks_per_record;
    uint16_t max_record_count;
    uint8_t reserved[BMS_DATA_LOG_INFO_SIZE - (sizeof(uint16_t) * 5)];
} BmsDataLogInfo, BmsEventLogInfo;

#endif