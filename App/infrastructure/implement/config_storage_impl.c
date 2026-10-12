#include <stdbool.h>
#include <string.h>

#include "../../domain/interface/config_storage.h"
#include "../../domain/interface/storage_common.h"
#include "../../domain/service/crc16_modbus.h"
#include "../service/nand_flash.h"

#define self (config_storage_private)

#define CONFIG_VERSION (APP_CONFIG_VERSION)
#define CONFIG_MAGIC (0x42434647)  // 'BCFG' in ASCII

#define BASE_BLOCK (NAND_FLASH_CONFIG_BASE_BLOCK)
#define BLOCK_COUNT (NAND_FLASH_CONFIG_BLOCK_COUNT)

#pragma pack(push, 1)

static struct {
    AppErrorInfo error_info;

    NandFlashAddress current_address;

    NandFlashInfo nand_info;
} config_storage_private;

typedef struct {
    uint32_t magic;
    BmsConfig config;  // BmsConfig already has version, timestamp and sequence
    uint16_t crc;      // CRC16-MODBUS
} MetaConfig;

#pragma pack(pop)

StorageResult _findLastConfig(MetaConfig* config) {
    if (config == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONFIG_STORAGE_INVALID_ARG);
        return STORAGE_RESULT_ERROR_INVALID_ARG;
    }

    uint16_t pages_per_block = self.nand_info.page_count_per_block;
    MetaConfig latest_config = {0};
    uint32_t latest_sequence = 0;
    bool found_valid = false;

    for (uint32_t block = BASE_BLOCK; block < BASE_BLOCK + BLOCK_COUNT; block++) {
        for (uint32_t page = 0; page < pages_per_block; page++) {
            NandFlashAddress address = {
                .block = block,
                .page = page,
            };

            MetaConfig temp_config;
            if (NandFlash_Read(&address, (uint8_t*)&temp_config, sizeof(MetaConfig)) !=
                APP_RESULT_OK) {
                continue;
            }

            if (temp_config.magic != CONFIG_MAGIC) {
                continue;
            }

            if (temp_config.config.meta.version != CONFIG_VERSION) {
                continue;
            }

            uint16_t calculated_crc = Crc16Modbus_Calculate(
                (uint8_t*)&temp_config, sizeof(MetaConfig) - sizeof(temp_config.crc));
            if (calculated_crc != temp_config.crc) {
                continue;
            }

            // find the meta data with the latest sequence number
            if (!found_valid || temp_config.config.meta.sequence > latest_sequence) {
                latest_config = temp_config;
                latest_sequence = temp_config.config.meta.sequence;
                found_valid = true;
                self.current_address = address;
            }
        }
    }

    if (found_valid) {
        *config = latest_config;
        return STORAGE_RESULT_OK;
    }

    return STORAGE_RESULT_ERROR_NOT_FOUND;
}

StorageResult _findLastEmptySpace(NandFlashAddress* address) {
    if (address == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONFIG_STORAGE_INVALID_ARG);
        return STORAGE_RESULT_ERROR_INVALID_ARG;
    }

    uint16_t pages_per_block = self.nand_info.page_count_per_block;

    for (uint32_t block = BASE_BLOCK; block < BASE_BLOCK + BLOCK_COUNT; block++) {
        for (uint32_t page = 0; page < pages_per_block; page++) {
            NandFlashAddress addr = {
                .block = block,
                .page = page,
            };

            MetaConfig temp_config;
            if (NandFlash_Read(&addr, (uint8_t*)&temp_config, sizeof(MetaConfig)) !=
                APP_RESULT_OK) {
                continue;
            }

            if (temp_config.magic != 0xFFFFFFFF) {  // empty state
                continue;
            }

            *address = addr;
            return STORAGE_RESULT_OK;
        }
    }

    return STORAGE_RESULT_ERROR_NOT_FOUND;
}

StorageResult _findOldestBlock(uint32_t* oldest_block) {
    if (oldest_block == NULL) {
        return STORAGE_RESULT_ERROR_INVALID_ARG;
    }

    uint16_t pages_per_block = self.nand_info.page_count_per_block;
    uint32_t oldest_sequence = 0xFFFFFFFF;
    uint32_t found_block = 0xFFFFFFFF;
    bool found_any = false;

    // Find the oldest sequence in each block
    for (uint32_t block = BASE_BLOCK; block < BASE_BLOCK + BLOCK_COUNT; block++) {
        uint32_t block_oldest_sequence = 0xFFFFFFFF;
        bool block_has_config = false;

        for (uint32_t page = 0; page < pages_per_block; page++) {
            NandFlashAddress address = {
                .block = block,
                .page = page,
            };

            MetaConfig temp_config;
            if (NandFlash_Read(&address, (uint8_t*)&temp_config, sizeof(MetaConfig)) !=
                APP_RESULT_OK) {
                continue;
            }

            if (temp_config.magic != CONFIG_MAGIC) {
                continue;
            }

            if (temp_config.config.meta.version != CONFIG_VERSION) {
                continue;
            }

            uint16_t calculated_crc = Crc16Modbus_Calculate(
                (uint8_t*)&temp_config, sizeof(MetaConfig) - sizeof(temp_config.crc));
            if (calculated_crc != temp_config.crc) {
                continue;
            }

            // Find oldest (minimum) sequence in this block
            if (temp_config.config.meta.sequence < block_oldest_sequence) {
                block_oldest_sequence = temp_config.config.meta.sequence;
                block_has_config = true;
            }
        }

        // Compare with global oldest
        if (block_has_config && block_oldest_sequence < oldest_sequence) {
            oldest_sequence = block_oldest_sequence;
            found_block = block;
            found_any = true;
        }
    }

    if (found_any) {
        *oldest_block = found_block;
        return STORAGE_RESULT_OK;
    }

    return STORAGE_RESULT_ERROR_NOT_FOUND;
}

StorageResult _garbageCollect(void) {
    // Find the block with oldest config
    uint32_t oldest_block;
    StorageResult result = _findOldestBlock(&oldest_block);
    if (result != STORAGE_RESULT_OK) {
        // No valid config found, just erase first block
        oldest_block = BASE_BLOCK;
    }

    // Erase only the oldest block
    NandFlashAddress erase_addr = {
        .block = oldest_block,
        .page = 0,
    };

    if (NandFlash_Erase(&erase_addr) != APP_RESULT_OK) {
        return STORAGE_RESULT_ERROR;
    }

    return STORAGE_RESULT_OK;
}

AppResult ConfigStorage_Initialize(void) {
    if (!NandFlash_IsInitialized()) {
        if (NandFlash_Initialize() != APP_RESULT_OK) {
            NandFlash_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    NandFlashInfo nand_info = {0};
    NandFlash_GetInfo(&nand_info);
    self.nand_info = nand_info;

    memset(&self.current_address, 0xFF, sizeof(self.current_address));

    return APP_RESULT_OK;
}

AppResult ConfigStorage_Deinitialize() { return APP_RESULT_OK; }

void ConfigStorage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult ConfigStorage_Load(BmsConfig* config) {
    MetaConfig meta_config = {0};

    StorageResult result = _findLastConfig(&meta_config);
    if (result == STORAGE_RESULT_ERROR_NOT_FOUND) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONFIG_STORAGE_EMPTY);
        return APP_RESULT_FAIL;

    } else if (result != STORAGE_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONFIG_STORAGE_FIND_CONFIG);
        return APP_RESULT_FAIL;
    }

    *config = meta_config.config;
    return APP_RESULT_OK;
}

AppResult ConfigStorage_Save(const BmsConfig* config) {
    if (config == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONFIG_STORAGE_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    bool is_first_write =
        (self.current_address.block == 0xFFFF && self.current_address.page == 0xFFFF);

    // Get current config to increment sequence
    MetaConfig meta_config = {0};
    meta_config.magic = CONFIG_MAGIC;
    meta_config.config = *config;

    if (is_first_write) {
        // First write - sequence starts at 1
        meta_config.config.meta.sequence = 1;
    } else {
        // Get current config to increment sequence
        MetaConfig current_config = {0};
        StorageResult result = _findLastConfig(&current_config);
        if (result == STORAGE_RESULT_OK) {
            meta_config.config.meta.sequence = current_config.config.meta.sequence + 1;
        } else {
            meta_config.config.meta.sequence = 1;
        }
    }

    // Calculate CRC
    meta_config.crc =
        Crc16Modbus_Calculate((uint8_t*)&meta_config, sizeof(MetaConfig) - sizeof(meta_config.crc));

    NandFlashAddress write_address;
    if (is_first_write) {
        // Find empty space for first write
        StorageResult result = _findLastEmptySpace(&write_address);
        if (result == STORAGE_RESULT_ERROR_NOT_FOUND) {
            if (_garbageCollect() != STORAGE_RESULT_OK) {
                APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONFIG_STORAGE_GC);
                return APP_RESULT_FAIL;
            }
            result = _findLastEmptySpace(&write_address);
            if (result != STORAGE_RESULT_OK) {
                return APP_RESULT_FAIL;
            }
        }
    } else {
        // Find next empty space after current config
        write_address = self.current_address;
        write_address.page++;

        // Check if we need to move to next block
        if (write_address.page >= self.nand_info.page_count_per_block) {
            write_address.page = 0;
            write_address.block++;

            // Check if we exceed config area
            if (write_address.block >= BASE_BLOCK + BLOCK_COUNT) {
                // Need garbage collection
                if (_garbageCollect() != STORAGE_RESULT_OK) {
                    APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONFIG_STORAGE_GC);
                    return APP_RESULT_FAIL;
                }

                // After GC, find empty space
                StorageResult result = _findLastEmptySpace(&write_address);
                if (result != STORAGE_RESULT_OK) {
                    APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONFIG_STORAGE_GC);
                    return APP_RESULT_FAIL;
                }
            }
        }

        // Verify the target location is empty
        MetaConfig temp_config;
        if (NandFlash_Read(&write_address, (uint8_t*)&temp_config, sizeof(MetaConfig)) ==
            APP_RESULT_OK) {
            if (temp_config.magic != 0xFFFFFFFF) {
                // Not empty, need to find proper empty space
                StorageResult result = _findLastEmptySpace(&write_address);
                if (result == STORAGE_RESULT_ERROR_NOT_FOUND) {
                    if (_garbageCollect() != STORAGE_RESULT_OK) {
                        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONFIG_STORAGE_GC);
                        return APP_RESULT_FAIL;
                    }
                    result = _findLastEmptySpace(&write_address);
                    if (result != STORAGE_RESULT_OK) {
                        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONFIG_STORAGE_GC);
                        return APP_RESULT_FAIL;
                    }
                }
            }
        }
    }

    // Write the config
    if (NandFlash_Write(&write_address, (uint8_t*)&meta_config, sizeof(MetaConfig)) !=
        APP_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONFIG_STORAGE_WRITE);
        return APP_RESULT_FAIL;
    }

    // Update current config address
    self.current_address = write_address;

    return APP_RESULT_OK;
}

AppResult ConfigStorage_EraseAll(void) {
    for (uint32_t block = BASE_BLOCK; block < BASE_BLOCK + BLOCK_COUNT; block++) {
        NandFlashAddress erase_addr = {
            .block = block,
            .page = 0,
        };

        if (NandFlash_Erase(&erase_addr) != APP_RESULT_OK) {
            NandFlash_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }
    return APP_RESULT_OK;
}