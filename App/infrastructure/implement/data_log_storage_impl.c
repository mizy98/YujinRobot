#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "../../domain/interface/data_log_storage.h"
#include "../../domain/interface/storage_common.h"
#include "../../domain/service/crc16_modbus.h"
#include "../service/nand_flash.h"

#define self (data_log_storage_private)

#define META_DATA_VERSION (1u)
#define META_DATA_MAGIC (0x444C4F47)  // 'DLOG' in ASCII

#define ENTRY_BASE_BLOCK (NAND_FLASH_DATA_LOG_ENTRY_BASE_BLOCK)
#define ENTRY_BLOCK_COUNT (NAND_FLASH_DATA_LOG_ENTRY_BLOCK_COUNT)

#define DATA_BASE_BLOCK (NAND_FLASH_DATA_BASE_BLOCK)

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint8_t version;
    uint32_t sequence;
    uint32_t day_id;  // (YYYYMMDD)
    DataEntryStatus status;
    NandFlashAddress start_address;
    NandFlashAddress end_address;
    uint32_t write_count;  // count of data log records written
    uint8_t reserved[2];
    uint16_t crc;  // CRC16-MODBUS
} DataLogEntry;
#pragma pack(pop)

static struct {
    AppErrorInfo error_info;

    uint8_t log_buffer[2048];
    uint16_t log_buffer_offset;

    DataLogEntry current_entry;

    NandFlashInfo nand_info;

    uint16_t data_unit_size;
    uint16_t block_count_per_record;
    uint16_t max_record_count;
    uint16_t total_block_count;  // = block_count_per_record * max_record_count

    uint8_t time_zone_offset_hours;

    DataLogStorageCallback callback;
} data_log_storage_private;

static uint32_t toDayIdFromTimestamp(uint32_t timestamp, uint8_t time_zone_offset_hours) {
    // Adjust timestamp for time zone offset
    int32_t adjusted_timestamp = (int32_t)timestamp + (time_zone_offset_hours * 3600);
    if (adjusted_timestamp < 0) {
        adjusted_timestamp = 0;
    }

    uint32_t days_since_epoch = adjusted_timestamp / 86400;

    uint32_t year = 1970;
    uint32_t remaining_days = days_since_epoch;

    // calculate year
    while (1) {
        uint32_t days_in_year = 365;

        // check leap year: divisible by 4 and (not divisible by 100 or divisible by 400)
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
            days_in_year = 366;
        }

        if (remaining_days < days_in_year) {
            break;
        }

        remaining_days -= days_in_year;
        year++;
    }

    // Month days table
    static const uint32_t month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Check if leap year
    uint32_t is_leap = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 1 : 0;

    // Calculate month/day
    uint32_t month = 1;
    for (uint32_t i = 0; i < 12; i++) {
        uint32_t days_in_month = month_days[i];
        if (i == 1 && is_leap) {  // February and leap year
            days_in_month = 29;
        }

        if (remaining_days < days_in_month) {
            month = i + 1;
            break;
        }

        remaining_days -= days_in_month;
    }

    uint32_t day = remaining_days + 1;

    return (year * 10000) + (month * 100) + day;
}

/*
static uint32_t toDayIdFromDate(uint16_t year, uint8_t month, uint8_t day) {
    if (month < 1 || month > 12 || day < 1 || day > 31) {
        return 0;
    }

    return (year * 10000) + (month * 100) + day;
}
*/

static uint32_t _getSequenceStartBlock(uint32_t sequence) {
    if (sequence == 0) {
        // Invalid sequence: return first valid block
        return DATA_BASE_BLOCK;
    }

    // Use modular arithmetic to map any sequence to 1 ~ max_record_count range
    uint32_t block_sequence = ((sequence - 1) % self.max_record_count) + 1;
    uint32_t start_block = DATA_BASE_BLOCK + (block_sequence - 1) * self.block_count_per_record;

    // Ensure we don't exceed the allocated data area
    uint32_t max_allowed_block = DATA_BASE_BLOCK + self.total_block_count - 1;
    if (start_block > max_allowed_block) {
        return DATA_BASE_BLOCK;  // Return safe default
    }

    return start_block;
}

static uint32_t _getSequenceEndBlock(uint32_t sequence) {
    if (sequence == 0) {
        // Invalid sequence: return first valid block end
        return DATA_BASE_BLOCK + self.block_count_per_record - 1;
    }

    uint32_t start_block = _getSequenceStartBlock(sequence);
    uint32_t end_block = start_block + self.block_count_per_record - 1;

    // Ensure we don't exceed the allocated data area
    uint32_t max_allowed_block = DATA_BASE_BLOCK + self.total_block_count - 1;
    if (end_block > max_allowed_block) {
        return max_allowed_block;  // Cap at maximum allowed
    }

    return end_block;
}

static uint32_t _getCurrentSequenceBlockCount(void) {
    if (self.current_entry.magic != META_DATA_MAGIC) {
        return 0;
    }

    return self.current_entry.end_address.block - self.current_entry.start_address.block + 1;
}

// A sequence's data blocks are reused (erased/overwritten) by later sequences once the
// modular block mapping wraps around (see _getSequenceStartBlock). A closed meta entry whose
// sequence already fell outside that window still exists in the meta log but no longer
// reflects what is physically on the data blocks it points to.
static bool _isEntryAlive(uint32_t sequence) {
    if (self.current_entry.magic != META_DATA_MAGIC || sequence == 0 ||
        sequence > self.current_entry.sequence) {
        return false;
    }

    return (self.current_entry.sequence - sequence) < self.max_record_count;
}

static StorageResult _findEmptyMetaSpace(NandFlashAddress* address) {
    if (address == NULL) {
        return STORAGE_RESULT_ERROR_INVALID_ARG;
    }

    uint16_t pages_per_block = self.nand_info.page_count_per_block;

    for (uint16_t block = ENTRY_BASE_BLOCK; block < ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT; block++) {
        for (uint16_t page = 0; page < pages_per_block; page++) {
            NandFlashAddress addr = {
                .block = block,
                .page = page,
            };
            DataLogEntry temp;

            if (NandFlash_Read(&addr, (uint8_t*)&temp, sizeof(temp)) != APP_RESULT_OK) {
                continue;
            }

            // check empty space (no magic number or all 0xFF) to find where to write new meta data
            if (temp.magic == 0xFFFFFFFF || temp.magic != META_DATA_MAGIC) {
                bool is_empty =
                    (temp.magic == 0xFFFFFFFF) ||
                    (temp.magic != META_DATA_MAGIC && temp.version == 0xFF && temp.status == 0xFF);

                if (is_empty) {
                    *address = addr;
                    return STORAGE_RESULT_OK;
                }
            }
        }
    }

    return STORAGE_RESULT_ERROR_STORAGE_FULL;
}

static StorageResult _findeLastEntry(DataLogEntry* meta_data) {
    if (meta_data == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_INVALID_META_DATA);
        return STORAGE_RESULT_ERROR_INVALID_ARG;
    }

    DataLogEntry latest_meta = {0};
    uint32_t latest_sequence = 0;
    bool found_valid = false;

    for (uint32_t block = ENTRY_BASE_BLOCK; block < ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT; block++) {
        uint32_t pages_per_block = self.nand_info.page_count_per_block;

        for (uint32_t page = 0; page < pages_per_block; page++) {
            NandFlashAddress address = {
                .block = block,
                .page = page,
            };

            DataLogEntry temp_meta;
            if (NandFlash_Read(&address, (uint8_t*)&temp_meta, sizeof(DataLogEntry)) !=
                APP_RESULT_OK) {
                continue;
            }

            if (temp_meta.magic != META_DATA_MAGIC) {
                continue;
            }

            uint16_t calculated_crc = Crc16Modbus_Calculate(
                (uint8_t*)&temp_meta, sizeof(DataLogEntry) - sizeof(temp_meta.crc));
            if (calculated_crc != temp_meta.crc) {
                continue;
            }

            // find the meta data with the latest sequence number
            if (!found_valid || temp_meta.sequence > latest_sequence) {
                latest_meta = temp_meta;
                latest_sequence = temp_meta.sequence;
                found_valid = true;
            }
        }
    }

    if (found_valid) {
        *meta_data = latest_meta;
        meta_data->write_count = _getCurrentSequenceBlockCount() *
                                 (self.nand_info.page_count_per_block / self.data_unit_size);
        return STORAGE_RESULT_OK;
    }

    return STORAGE_RESULT_ERROR_NOT_FOUND;
}

static StorageResult _findLastRecord(DataLogEntry* meta_data, NandFlashAddress* record_address) {
    uint32_t start_block = meta_data->start_address.block;
    uint32_t end_block;

    bool is_opened = (meta_data->status == META_DATA_STATUS_OPENED);

    if (is_opened) {
        end_block = _getSequenceEndBlock(meta_data->sequence);
    } else {
        end_block = meta_data->end_address.block;
    }

    // Search from end to start to avoid underflow
    for (uint32_t i = 0; i <= (end_block - start_block); i++) {
        uint32_t block = end_block - i;  // Safe reverse iteration
        uint32_t pages_per_block = self.nand_info.page_count_per_block;

        for (int32_t page = (block == end_block) ? meta_data->end_address.page
                                                 : (pages_per_block - 1);
             page >= 0; page--) {
            NandFlashAddress addr = {
                .block = block,
                .page = page,
            };

            BmsBriefData temp;
            if (NandFlash_Read(&addr, (uint8_t*)&temp, sizeof(temp)) != APP_RESULT_OK) {
                continue;
            }

            // check empty space (all 0xFF) to find the last record
            if (temp.timestamp != 0xFFFFFFFF) {
                *record_address = addr;
                return STORAGE_RESULT_OK;
            }
        }
    }

    return STORAGE_RESULT_ERROR_NOT_FOUND;
}

/*
static StorageResult _getMaxSequenceMetaDataInBlock(uint32_t block, DataLogEntry* meta_data) {
    uint32_t pages_per_block = self.nand_info.page_count_per_block;
    DataLogEntry max_sequence_entry = {0};

    for (uint32_t page = 0; page < pages_per_block; page++) {
        NandFlashAddress addr = {
            .block = block,
            .page = page,
        };

        DataLogEntry temp;
        if (NandFlash_Read(&addr, (uint8_t*)&temp, sizeof(temp)) != APP_RESULT_OK) {
            continue;
        }

        if (temp.magic != META_DATA_MAGIC) {
            continue;
        }

        uint16_t crc =
            Crc16Modbus_Calculate((uint8_t*)&temp, sizeof(DataLogEntry) - sizeof(temp.crc));

        if (crc != temp.crc) {
            continue;
        }

        if (temp.sequence > max_sequence_entry.sequence) {
            max_sequence_entry = temp;
        }
    }

    if (meta_data != NULL) {
        *meta_data = max_sequence_entry;
    }

    return STORAGE_RESULT_OK;
}
*/

static StorageResult _garbageCollectMetaData(void) {
    // 최신 데이터로 부터 valid (closed된 상태) meta data를 최대 max_record_count 만큼 찾아서 신규
    // meta data로 복사하고, 나머지 meta data는 블록 단위로 지우는 방식으로 가비지 컬렉션 수행

    // 1. 모든 valid한 CLOSED 메타데이터 수집
    DataLogEntry valid_entries[128];
    uint8_t valid_count = 0;

    uint32_t pages_per_block = self.nand_info.page_count_per_block;

    // 모든 메타 블록에서 valid한 메타데이터 수집
    for (uint32_t block = ENTRY_BASE_BLOCK; block < ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT; block++) {
        for (uint32_t page = 0; page < pages_per_block; page++) {
            NandFlashAddress address = {
                .block = block,
                .page = page,
            };

            DataLogEntry temp_meta;
            if (NandFlash_Read(&address, (uint8_t*)&temp_meta, sizeof(DataLogEntry)) !=
                APP_RESULT_OK) {
                continue;
            }

            if (temp_meta.magic != META_DATA_MAGIC) {
                continue;
            }

            uint16_t calculated_crc = Crc16Modbus_Calculate(
                (uint8_t*)&temp_meta, sizeof(DataLogEntry) - sizeof(temp_meta.crc));
            if (calculated_crc != temp_meta.crc) {
                continue;
            }

            if (temp_meta.status == META_DATA_STATUS_CLOSED && valid_count < 128) {
                valid_entries[valid_count] = temp_meta;
                valid_count++;
            }
        }
    }

    // 2. sequence 번호로 정렬 (최신순 - 내림차순)
    for (uint8_t i = 0; valid_count > 0 && i < valid_count - 1; i++) {
        for (uint8_t j = i + 1; j < valid_count; j++) {
            if (valid_entries[i].sequence < valid_entries[j].sequence) {
                DataLogEntry temp = valid_entries[i];
                valid_entries[i] = valid_entries[j];
                valid_entries[j] = temp;
            }
        }
    }

    // 3. 최대 max_record_count개만 보존할 항목 선택
    uint8_t preserve_count =
        (valid_count < self.max_record_count) ? valid_count : self.max_record_count;

    // 4. 모든 메타 블록 삭제
    for (uint32_t block = ENTRY_BASE_BLOCK; block < ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT; block++) {
        NandFlashAddress erase_addr = {
            .block = block,
            .page = 0,
        };

        if (NandFlash_Erase(&erase_addr) != APP_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }
    }

    // 5. 선택된 메타데이터들을 다시 기록
    uint32_t current_block = ENTRY_BASE_BLOCK;
    uint32_t current_page = 0;

    for (uint8_t i = 0; i < preserve_count; i++) {
        NandFlashAddress write_addr = {
            .block = current_block,
            .page = current_page,
        };

        // CRC 재계산
        valid_entries[i].crc = Crc16Modbus_Calculate(
            (uint8_t*)&valid_entries[i], sizeof(DataLogEntry) - sizeof(valid_entries[i].crc));

        if (NandFlash_Write(&write_addr, (uint8_t*)&valid_entries[i], sizeof(DataLogEntry)) !=
            APP_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }

        // 다음 위치로 이동
        current_page++;
        if (current_page >= self.nand_info.page_count_per_block) {
            current_page = 0;
            current_block++;

            // 메타 블록 범위 초과 체크
            if (current_block >= ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT) {
                break;  // 더 이상 쓸 공간이 없음
            }
        }
    }

    return STORAGE_RESULT_OK;
}

static StorageResult _appendMetaData(DataLogEntry* meta_data) {
    NandFlashAddress address;

    StorageResult result = _findEmptyMetaSpace(&address);
    if (result == STORAGE_RESULT_ERROR_STORAGE_FULL) {
        if (_garbageCollectMetaData() != STORAGE_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }
        result = _findEmptyMetaSpace(&address);
    }

    if (result != STORAGE_RESULT_OK) {
        return STORAGE_RESULT_ERROR;
    }

    meta_data->crc =
        Crc16Modbus_Calculate((uint8_t*)meta_data, sizeof(DataLogEntry) - sizeof(meta_data->crc));

    if (NandFlash_Write(&address, (uint8_t*)meta_data, sizeof(DataLogEntry)) != APP_RESULT_OK) {
        return STORAGE_RESULT_ERROR;
    }

    return STORAGE_RESULT_OK;
}

static StorageResult _writeDataLog(void) {
    if (NandFlash_Write(&self.current_entry.end_address, self.log_buffer, self.log_buffer_offset) !=
        APP_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_DRV_WRITE);
        return STORAGE_RESULT_ERROR;
    }

    if (self.callback.onRecordStored != NULL) {
        self.callback.onRecordStored((BmsBriefData*)(self.log_buffer),
                                     self.log_buffer_offset / self.data_unit_size);
    }

    NandFlashAddress* addr = &self.current_entry.end_address;
    ++addr->page;
    if (addr->page >= self.nand_info.page_count_per_block) {
        addr->page = 0;
        addr->block++;

        // Check sequence area boundary
        uint32_t sequence_end_block = _getSequenceEndBlock(self.current_entry.sequence);
        if (addr->block > sequence_end_block) {
            // Reached sequence boundary, will need new sequence
            return STORAGE_RESULT_ERROR_SEQUENCE_FULL;
        }

        // Check for bad blocks and find next good block within sequence
        while (addr->block <= sequence_end_block) {
            bool is_bad = false;
            if (NandFlash_CheckBadBlock(addr->block, &is_bad) == APP_RESULT_OK && !is_bad) {
                break;  // Found a good block
            }

            // Skip bad block or check error - try next block
            addr->block++;
        }

        // If no good block found within sequence range
        if (addr->block > sequence_end_block) {
            return STORAGE_RESULT_ERROR_SEQUENCE_FULL;
        }

        addr->page = 0;
    }

    self.current_entry.write_count += (self.log_buffer_offset / self.data_unit_size);

    return STORAGE_RESULT_OK;
}

static StorageResult _startNewSequence(uint32_t day_id) {
    // Close current buffer
    if (self.log_buffer_offset > 0) {
        if (_writeDataLog() != STORAGE_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }
        self.log_buffer_offset = 0;
    }

    // Close current meta data
    if (self.current_entry.magic == META_DATA_MAGIC &&
        self.current_entry.status == META_DATA_STATUS_OPENED) {
        self.current_entry.status = META_DATA_STATUS_CLOSED;

        if (_appendMetaData(&self.current_entry) != STORAGE_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }
    }

    // Create new sequence meta data with sequence numbering
    DataLogEntry meta_data = {0};
    meta_data.magic = META_DATA_MAGIC;
    meta_data.version = META_DATA_VERSION;

    // Implement sequence numbering for wear leveling
    if (self.current_entry.magic == META_DATA_MAGIC) {
        // Sequence keeps increasing, block allocation uses modular arithmetic
        meta_data.sequence = self.current_entry.sequence + 1;
    } else {
        meta_data.sequence = 1;  // Start from sequence 1
    }

    meta_data.day_id = day_id;
    meta_data.status = META_DATA_STATUS_OPENED;

    // Allocate fixed area for new sequence
    uint32_t new_start_block = _getSequenceStartBlock(meta_data.sequence);
    uint32_t new_end_block = _getSequenceEndBlock(meta_data.sequence);

    // Erase the allocated block range to prevent data collision
    for (uint32_t block = new_start_block; block <= new_end_block; block++) {
        NandFlashAddress erase_addr = {
            .block = block,
            .page = 0,
        };

        if (NandFlash_Erase(&erase_addr) != APP_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }
    }

    meta_data.start_address.block = new_start_block;
    meta_data.start_address.page = 0;
    meta_data.end_address.block = new_start_block;
    meta_data.end_address.page = 0;
    meta_data.write_count = 0;

    if (_appendMetaData(&meta_data) != STORAGE_RESULT_OK) {
        return STORAGE_RESULT_ERROR;
    }

    self.current_entry = meta_data;
    return STORAGE_RESULT_OK;
}

static StorageResult _startNewDay(uint32_t day_id) {
    if (self.log_buffer_offset > 0) {
        StorageResult result = _writeDataLog();
        if (result == STORAGE_RESULT_ERROR) {
            return STORAGE_RESULT_ERROR;
        }
        self.log_buffer_offset = 0;
    }

    if (self.current_entry.magic == META_DATA_MAGIC &&
        self.current_entry.status == META_DATA_STATUS_OPENED) {
        self.current_entry.status = META_DATA_STATUS_CLOSED;

        if (_appendMetaData(&self.current_entry) != STORAGE_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }
    }

    DataLogEntry meta_data = {0};

    meta_data.magic = META_DATA_MAGIC;
    meta_data.version = META_DATA_VERSION;

    // Implement sequence numbering for wear leveling
    if (self.current_entry.magic == META_DATA_MAGIC) {
        // Sequence keeps increasing, block allocation uses modular arithmetic
        meta_data.sequence = self.current_entry.sequence + 1;
    } else {
        meta_data.sequence = 1;  // Start from sequence 1
    }

    meta_data.day_id = day_id;
    meta_data.status = META_DATA_STATUS_OPENED;

    // Set address for first time (when no valid meta data found)
    if (self.current_entry.magic != META_DATA_MAGIC) {
        uint32_t new_start_block = _getSequenceStartBlock(1);  // sequence 1
        uint32_t new_end_block = _getSequenceEndBlock(1);

        // Erase the allocated block range to prevent data collision
        for (uint32_t block = new_start_block; block <= new_end_block; block++) {
            NandFlashAddress erase_addr = {
                .block = block,
                .page = 0,
            };

            if (NandFlash_Erase(&erase_addr) != APP_RESULT_OK) {
                return STORAGE_RESULT_ERROR;
            }
        }

        meta_data.start_address.block = new_start_block;
        meta_data.start_address.page = 0;
        meta_data.end_address.block = new_start_block;
        meta_data.end_address.page = 0;
    } else {
        // Allocate new sequence area
        uint32_t new_start_block = _getSequenceStartBlock(meta_data.sequence);
        uint32_t new_end_block = _getSequenceEndBlock(meta_data.sequence);

        // Erase the allocated block range to prevent data collision
        for (uint32_t block = new_start_block; block <= new_end_block; block++) {
            NandFlashAddress erase_addr = {
                .block = block,
                .page = 0,
            };

            if (NandFlash_Erase(&erase_addr) != APP_RESULT_OK) {
                return STORAGE_RESULT_ERROR;
            }
        }

        meta_data.start_address.block = new_start_block;
        meta_data.start_address.page = 0;
        meta_data.end_address.block = new_start_block;
        meta_data.end_address.page = 0;
    }
    meta_data.write_count = 0;

    if (_appendMetaData(&meta_data) != STORAGE_RESULT_OK) {
        return STORAGE_RESULT_ERROR;
    }

    self.current_entry = meta_data;

    return STORAGE_RESULT_OK;
}

AppResult DataLogStorage_Initialize(const DataLogStorageCallback* callback, uint32_t timestamp,
                                    uint8_t time_zone_offset_hours) {
    if (callback == NULL || callback->onRecordLoaded == NULL ||
        callback->onRecordLoadFinished == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_INVALID_CALLBACK);
        return APP_RESULT_INVALID_ARG;
    }

    if (time_zone_offset_hours > 14) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    self.time_zone_offset_hours = time_zone_offset_hours;

    if (!NandFlash_IsInitialized()) {
        if (NandFlash_Initialize() != APP_RESULT_OK) {
            NandFlash_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    NandFlashInfo nand_info = {0};
    NandFlash_GetInfo(&nand_info);

    if (sizeof(self.log_buffer) < nand_info.page_size) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_LOG_BUFFER_TOO_SMALL);
        return APP_RESULT_FAIL;
    }

    self.nand_info = nand_info;

    if (sizeof(BmsBriefData) > NAND_FLASH_LARGE_DATA_UNIT_SIZE) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_TOO_LARGE_DATA_UNIT);
        return APP_RESULT_FAIL;

    } else if (sizeof(BmsBriefData) > NAND_FLASH_SMALL_DATA_UNIT_SIZE) {
        self.data_unit_size = NAND_FLASH_LARGE_DATA_UNIT_SIZE;
        self.block_count_per_record = NAND_FLASH_LARGE_DATA_COUNT_PER_META;
        self.max_record_count = NAND_FLASH_LARGE_DATA_MAX_COUNT;

    } else {
        self.data_unit_size = NAND_FLASH_SMALL_DATA_UNIT_SIZE;
        self.block_count_per_record = NAND_FLASH_SMALL_DATA_COUNT_PER_META;
        self.max_record_count = NAND_FLASH_SMALL_DATA_MAX_COUNT;
    }

    self.total_block_count = self.block_count_per_record * self.max_record_count;

    memset(&self.current_entry, 0xFF, sizeof(DataLogEntry));

    StorageResult result = _findeLastEntry(&self.current_entry);
    if ((result != STORAGE_RESULT_OK) && (result != STORAGE_RESULT_ERROR_NOT_FOUND)) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_FIND_META_DATA);
        return APP_RESULT_FAIL;

    } else if (result == STORAGE_RESULT_OK) {
        // Check timestamp to determine if we need to start a new day
        uint32_t current_day_id = toDayIdFromTimestamp(timestamp, time_zone_offset_hours);

        // If different day, start new day sequence
        if (self.current_entry.day_id != current_day_id) {
            if (_startNewDay(current_day_id) != STORAGE_RESULT_OK) {
                return APP_RESULT_FAIL;
            }
        }

        // Find last record to set correct end address for appending new data
        NandFlashAddress last_record_address;
        if (_findLastRecord(&self.current_entry, &last_record_address) == STORAGE_RESULT_OK) {
            self.current_entry.end_address = last_record_address;

            // Move to next page for new record
            NandFlashAddress* address = &self.current_entry.end_address;
            ++address->page;
            if (address->page >= self.nand_info.page_count_per_block) {
                address->page = 0;
                address->block++;
            }
        } else {
            // No valid record found, start from meta data's start address
            self.current_entry.end_address = self.current_entry.start_address;
        }

        uint16_t written_page_count =
            (self.current_entry.end_address.block - self.current_entry.start_address.block) *
                self.nand_info.page_count_per_block +
            self.current_entry.end_address.page - self.current_entry.start_address.page;
        uint16_t record_count_per_page = self.nand_info.page_size / self.data_unit_size;

        self.current_entry.write_count = written_page_count * record_count_per_page;
    }

    self.log_buffer_offset = 0;  // Initialize log buffer offset
    self.callback = *callback;

    return APP_RESULT_OK;
}

AppResult DataLogStorage_Deinitialize(void) { return APP_RESULT_OK; }

AppResult DataLogStorage_GetInfo(BmsDataLogInfo* info) {
    if (info == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    info->record_unit_size = self.data_unit_size;
    info->page_size = self.nand_info.page_size;
    info->pages_per_block = self.nand_info.page_count_per_block;
    info->blocks_per_record = self.block_count_per_record;
    info->max_record_count = self.max_record_count;

    return APP_RESULT_OK;
}

AppResult DataLogStorage_GetCurrentEntry(BmsDataLogEntry* entry) {
    if (entry == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    if (self.current_entry.magic != META_DATA_MAGIC) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_FIND_META_DATA);
        return APP_RESULT_FAIL;
    }

    entry->day_id = self.current_entry.day_id;
    entry->sequence = self.current_entry.sequence;
    entry->record_count = (uint16_t)self.current_entry.write_count;
    entry->start_address.block = self.current_entry.start_address.block;
    entry->start_address.page = self.current_entry.start_address.page;
    entry->end_address.block = self.current_entry.end_address.block;
    entry->end_address.page = self.current_entry.end_address.page;

    return APP_RESULT_OK;
}

void DataLogStorage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult DataLogStorage_FindClosedEntryByStartAddress(uint16_t block, uint16_t page,
                                                       BmsDataLogEntry* entry) {
    if (entry == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    for (uint32_t b = ENTRY_BASE_BLOCK; b < ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT; b++) {
        uint32_t pages_per_block = self.nand_info.page_count_per_block;

        for (uint32_t p = 0; p < pages_per_block; p++) {
            NandFlashAddress address = {.block = b, .page = p};
            DataLogEntry temp;

            if (NandFlash_Read(&address, (uint8_t*)&temp, sizeof(DataLogEntry)) != APP_RESULT_OK) {
                continue;
            }

            if (temp.magic != META_DATA_MAGIC || temp.status != META_DATA_STATUS_CLOSED) {
                continue;
            }

            uint16_t crc =
                Crc16Modbus_Calculate((uint8_t*)&temp, sizeof(DataLogEntry) - sizeof(temp.crc));
            if (crc != temp.crc) {
                continue;
            }

            if (!_isEntryAlive(temp.sequence)) {
                continue;
            }

            if (temp.start_address.block == block && temp.start_address.page == page) {
                entry->day_id = temp.day_id;
                entry->sequence = temp.sequence;
                entry->record_count = (uint16_t)temp.write_count;
                entry->start_address.block = temp.start_address.block;
                entry->start_address.page = temp.start_address.page;
                entry->end_address.block = temp.end_address.block;
                entry->end_address.page = temp.end_address.page;
                return APP_RESULT_OK;
            }
        }
    }

    APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_FIND_META_DATA);
    return APP_RESULT_FAIL;
}

AppResult DataLogStorage_GetClosedEntries(BmsDataLogEntry* meta_datas, const uint8_t request_count,
                                          uint8_t* result_count) {
    if (meta_datas == NULL || result_count == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    if (request_count == 0) {
        *result_count = 0;
        return APP_RESULT_OK;
    }

    DataLogEntry closed_entries[128];
    uint8_t found_count = 0;
    const uint8_t max_capacity = sizeof(closed_entries) / sizeof(closed_entries[0]);

    // Scan the whole meta log (not just until `request_count` matches are found) so the
    // sort below picks the true latest entries rather than whichever were encountered first.
    for (uint32_t i = 0; i < ENTRY_BLOCK_COUNT && found_count < max_capacity; i++) {
        uint32_t pages_per_block = self.nand_info.page_count_per_block;

        for (uint32_t page = 0; page < pages_per_block && found_count < max_capacity; page++) {
            NandFlashAddress address = {
                .block = ENTRY_BASE_BLOCK + i,
                .page = page,
            };

            DataLogEntry temp_meta;
            if (NandFlash_Read(&address, (uint8_t*)&temp_meta, sizeof(DataLogEntry)) !=
                APP_RESULT_OK) {
                continue;
            }

            if (temp_meta.magic != META_DATA_MAGIC) {
                continue;
            }

            uint16_t calculated_crc = Crc16Modbus_Calculate(
                (uint8_t*)&temp_meta, sizeof(DataLogEntry) - sizeof(temp_meta.crc));
            if (calculated_crc != temp_meta.crc) {
                continue;
            }

            // Skip entries whose data blocks have already been overwritten by a later
            // sequence that wrapped around the ring buffer (see _isEntryAlive).
            if (temp_meta.status == META_DATA_STATUS_CLOSED && _isEntryAlive(temp_meta.sequence)) {
                closed_entries[found_count] = temp_meta;
                found_count++;
            }
        }
    }

    // bubble sort to sort by sequence in descending order (latest first)
    for (uint8_t i = 0; found_count > 0 && i < found_count - 1; i++) {
        for (uint8_t j = i + 1; j < found_count; j++) {
            if (closed_entries[i].sequence < closed_entries[j].sequence) {
                DataLogEntry temp = closed_entries[i];
                closed_entries[i] = closed_entries[j];
                closed_entries[j] = temp;
            }
        }
    }

    uint8_t return_count = (found_count < request_count) ? found_count : request_count;

    BmsDataLogEntry* result_array = (BmsDataLogEntry*)meta_datas;

    for (uint8_t i = 0; i < return_count; i++) {
        result_array[i].day_id = closed_entries[i].day_id;
        result_array[i].sequence = closed_entries[i].sequence;
        result_array[i].record_count = closed_entries[i].write_count;
        result_array[i].start_address.block = closed_entries[i].start_address.block;
        result_array[i].start_address.page = closed_entries[i].start_address.page;
        result_array[i].end_address.block = closed_entries[i].end_address.block;
        result_array[i].end_address.page = closed_entries[i].end_address.page;
    }

    *result_count = return_count;

    return APP_RESULT_OK;
}

AppResult DataLogStorage_Record(const BmsBriefData* data, float* usage) {
    if (data == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_INVALID_DATA);
        return APP_RESULT_INVALID_ARG;
    }

    uint32_t day_id = toDayIdFromTimestamp(data->timestamp, self.time_zone_offset_hours);

    // Check day change
    if (self.current_entry.magic != META_DATA_MAGIC || self.current_entry.day_id != day_id) {
        if (_startNewDay(day_id) != STORAGE_RESULT_OK) {
            return APP_RESULT_FAIL;
        }
    }
    // Check sequence block limit
    else if (_getCurrentSequenceBlockCount() >= self.block_count_per_record) {
        if (_startNewSequence(day_id) != STORAGE_RESULT_OK) {
            return APP_RESULT_FAIL;
        }
    }

    memcpy(self.log_buffer + self.log_buffer_offset, data, self.data_unit_size);

    self.log_buffer_offset += self.data_unit_size;

    if (self.log_buffer_offset + self.data_unit_size > self.nand_info.page_size) {
        StorageResult result = _writeDataLog();
        if (result == STORAGE_RESULT_ERROR_SEQUENCE_FULL) {
            self.log_buffer_offset = 0;

            if (_startNewSequence(day_id) != STORAGE_RESULT_OK) {
                return APP_RESULT_FAIL;
            }

            // After starting new sequence, write the log buffer again
            self.log_buffer_offset = self.nand_info.page_size;
            if (_writeDataLog() != STORAGE_RESULT_OK) {
                return APP_RESULT_FAIL;
            }
        } else if (result != STORAGE_RESULT_OK) {
            return APP_RESULT_FAIL;
        }

        self.log_buffer_offset = 0;
    }

    if (usage != NULL) {
        if (self.current_entry.magic == META_DATA_MAGIC) {
            // Calculate usage based on current sequence (including page-level precision)
            uint32_t used_blocks = _getCurrentSequenceBlockCount();
            uint32_t pages_per_block = self.nand_info.page_count_per_block;
            uint32_t total_pages = self.block_count_per_record * pages_per_block;

            // Calculate used pages more precisely
            uint32_t used_pages = 0;
            if (used_blocks > 0) {
                // Full blocks contribute all their pages
                used_pages = (used_blocks - 1) * pages_per_block;
                // Add pages used in the current partial block
                used_pages += (self.current_entry.end_address.page + 1);
            }

            *usage = (float)used_pages / total_pages;
        } else {
            *usage = 0.0f;
        }
    }

    return APP_RESULT_OK;
}

AppResult DataLogStorage_EraseAllRecords(void) {
    // Erase all meta data blocks
    for (uint32_t block = ENTRY_BASE_BLOCK; block < ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT; block++) {
        NandFlashAddress erase_addr = {
            .block = block,
            .page = 0,
        };

        if (NandFlash_Erase(&erase_addr) != APP_RESULT_OK) {
            NandFlash_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    // Erase all data blocks
    for (uint32_t block = DATA_BASE_BLOCK; block < DATA_BASE_BLOCK + self.total_block_count;
         block++) {
        NandFlashAddress erase_addr = {
            .block = block,
            .page = 0,
        };

        if (NandFlash_Erase(&erase_addr) != APP_RESULT_OK) {
            NandFlash_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    // Reset current meta data state
    memset(&self.current_entry, 0xFF, sizeof(DataLogEntry));
    self.log_buffer_offset = 0;

    return APP_RESULT_OK;
}

AppResult DataLogStorage_StartReadStream(const BmsDataLogEntry* meta_data) {
    if (meta_data == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    uint16_t records_per_page = self.nand_info.page_size / self.data_unit_size;
    uint32_t records_remaining = meta_data->record_count;

    NandFlashAddress addr = {
        .block = meta_data->start_address.block,
        .page = meta_data->start_address.page,
    };

    while (records_remaining > 0) {
        if (NandFlash_Read(&addr, self.log_buffer, self.nand_info.page_size) != APP_RESULT_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_DRV_READ);
            return APP_RESULT_FAIL;
        }

        uint16_t count =
            (records_remaining < records_per_page) ? (uint16_t)records_remaining : records_per_page;

        if (self.callback.onRecordLoaded != NULL) {
            self.callback.onRecordLoaded((BmsBriefData*)self.log_buffer, count);
        }

        records_remaining -= count;

        addr.page++;
        if (addr.page >= self.nand_info.page_count_per_block) {
            addr.page = 0;
            addr.block++;
        }
    }

    if (self.callback.onRecordLoadFinished != NULL) {
        self.callback.onRecordLoadFinished();
    }

    return APP_RESULT_OK;
}

AppResult DataLogStorage_ReadRawData(uint8_t* data, uint16_t size, uint16_t block, uint16_t page) {
    if (block >= self.nand_info.block_count || page >= self.nand_info.page_count_per_block) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    NandFlashAddress address = {
        .block = block,
        .page = page,
    };

    if (NandFlash_Read(&address, data, size) != APP_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_DATA_LOG_STORAGE_DRV_READ);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}