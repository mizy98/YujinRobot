#ifndef NAND_FLASH_H
#define NAND_FLASH_H

#include <stdbool.h>

#include "../../domain/model/app_model.h"
#include "platform.h"

#define NAND_FLASH_CONFIG_BASE_BLOCK (0u)
#define NAND_FLASH_CONFIG_BLOCK_COUNT (3u)

#define NAND_FLASH_EVENT_LOG_ENTRY_BASE_BLOCK \
    (NAND_FLASH_CONFIG_BASE_BLOCK + NAND_FLASH_CONFIG_BLOCK_COUNT)
#define NAND_FLASH_EVENT_LOG_ENTRY_BLOCK_COUNT (2u)

#define NAND_FLASH_EVENT_LOG_BASE_BLOCK \
    (NAND_FLASH_EVENT_LOG_ENTRY_BASE_BLOCK + NAND_FLASH_EVENT_LOG_ENTRY_BLOCK_COUNT)
#define NAND_FLASH_EVENT_LOG_BLOCK_COUNT (3u)

#define NAND_FLASH_EVENT_LOG_BUFFER_BASE_BLOCK \
    (NAND_FLASH_EVENT_LOG_BASE_BLOCK + NAND_FLASH_EVENT_LOG_BLOCK_COUNT)
#define NAND_FLASH_EVENT_LOG_BUFFER_BLOCK_COUNT (1u)

#define NAND_FLASH_DATA_LOG_ENTRY_BASE_BLOCK \
    (NAND_FLASH_EVENT_LOG_BUFFER_BASE_BLOCK + NAND_FLASH_EVENT_LOG_BUFFER_BLOCK_COUNT)
#define NAND_FLASH_DATA_LOG_ENTRY_BLOCK_COUNT (3u)

#define NAND_FLASH_LARGE_DATA_UNIT_SIZE (128u)
#define NAND_FLASH_LARGE_DATA_COUNT_PER_META (100u)
#define NAND_FLASH_LARGE_DATA_MAX_COUNT (7u)

#define NAND_FLASH_SMALL_DATA_UNIT_SIZE (64u)
#define NAND_FLASH_SMALL_DATA_COUNT_PER_META (50u)
#define NAND_FLASH_SMALL_DATA_MAX_COUNT (14u)

#define NAND_FLASH_DATA_BASE_BLOCK \
    (NAND_FLASH_DATA_LOG_ENTRY_BASE_BLOCK + NAND_FLASH_DATA_LOG_ENTRY_BLOCK_COUNT)

#pragma pack(push, 1)
typedef struct {
    uint16_t block;
    uint16_t page;
} NandFlashAddress;

typedef struct {
    uint16_t block_count;
    uint16_t page_count_per_block;
    uint16_t page_size;
} NandFlashInfo;
#pragma pack(pop)

AppResult NandFlash_Initialize(void);
AppResult NandFlash_Deinitialize(void);
void NandFlash_GetLastErrorInfo(AppErrorInfo* info);
bool NandFlash_IsInitialized(void);
void NandFlash_GetInfo(NandFlashInfo* info);
AppResult NandFlash_Write(const NandFlashAddress* address, const uint8_t* data, uint16_t size);
AppResult NandFlash_Read(const NandFlashAddress* address, uint8_t* data, uint16_t size);
AppResult NandFlash_Erase(const NandFlashAddress* address);
AppResult NandFlash_CheckBadBlock(uint16_t block, bool* is_bad);

#endif