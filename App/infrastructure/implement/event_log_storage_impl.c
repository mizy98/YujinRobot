#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "../../domain/interface/event_log_storage.h"
#include "../../domain/interface/storage_common.h"
#include "../../domain/service/crc16_modbus.h"
#include "../service/nand_flash.h"

#define self (event_log_storage_private)

#define EVENT_LOG_VERSION (1u)
#define EVENT_LOG_MAGIC (0x4556544C)  // 'EVTL' in ASCII

#define EVENT_LOG_EDR_MAX_COUNT (21u)

#define ENTRY_BASE_BLOCK (NAND_FLASH_EVENT_LOG_ENTRY_BASE_BLOCK)
#define ENTRY_BLOCK_COUNT (NAND_FLASH_EVENT_LOG_ENTRY_BLOCK_COUNT)

#define DATA_BASE_BLOCK (NAND_FLASH_EVENT_LOG_BASE_BLOCK)
#define DATA_BLOCK_COUNT (NAND_FLASH_EVENT_LOG_BLOCK_COUNT)
#define BUFFER_BASE_BLOCK (NAND_FLASH_EVENT_LOG_BUFFER_BASE_BLOCK)

#define EVENT_LOG_PAGE_BUF_SIZE (2048u)

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint64_t datetime_id;  // (YYYYMMDDHHMMSS)
    uint8_t type;
    uint8_t version;
    uint32_t sequence;
    uint32_t record_count;
    NandFlashAddress start_address;
    NandFlashAddress end_address;
    uint32_t reset_flag;
    uint32_t protection_flag;
    int32_t protection_snapshot_value;
    int32_t protection_snapshot_threshold;
    uint16_t crc;  // CRC16-MODBUS
} EventLogEntry;

typedef struct {
    EventLogEntry entry;
    BmsBriefData records[EVENT_LOG_EDR_MAX_COUNT];
} PreservedEventLog;

CCMRAM static struct {
    AppErrorInfo error_info;

    EventLogEntry current_entry;

    NandFlashInfo nand_info;

    uint8_t time_zone_offset_hours;
} event_log_storage_private;

#pragma pack(pop)

static StorageResult _appendEntry(EventLogEntry* entry);
static StorageResult _garbageCollectPreservingLatest(uint8_t preserve_limit);

static uint32_t _pagesPerDataArea(void) {
    return (uint32_t)self.nand_info.page_count_per_block * DATA_BLOCK_COUNT;
}

static uint16_t _requiredPageCount(uint32_t data_size) {
    uint16_t page_size = self.nand_info.page_size;
    return (uint16_t)((data_size + page_size - 1u) / page_size);
}

static uint64_t _toDateTimeIdFromTimestamp(uint32_t timestamp, uint8_t time_zone_offset_hours) {
    timestamp += (uint32_t)time_zone_offset_hours * 3600u;

    uint32_t days_since_epoch = timestamp / 86400u;
    uint32_t seconds_of_day = timestamp % 86400u;

    uint32_t year = 1970u;
    uint32_t remaining_days = days_since_epoch;

    while (1) {
        uint32_t days_in_year = 365u;
        if ((year % 4u == 0u && year % 100u != 0u) || (year % 400u == 0u)) {
            days_in_year = 366u;
        }

        if (remaining_days < days_in_year) {
            break;
        }

        remaining_days -= days_in_year;
        year++;
    }

    static const uint8_t month_days[12] = {31u, 28u, 31u, 30u, 31u, 30u,
                                           31u, 31u, 30u, 31u, 30u, 31u};

    uint8_t is_leap = ((year % 4u == 0u && year % 100u != 0u) || (year % 400u == 0u)) ? 1u : 0u;

    uint32_t month = 1u;
    for (uint32_t i = 0; i < 12u; i++) {
        uint32_t days_in_month = month_days[i];
        if (i == 1u && is_leap == 1u) {
            days_in_month = 29u;
        }

        if (remaining_days < days_in_month) {
            month = i + 1u;
            break;
        }

        remaining_days -= days_in_month;
    }

    uint32_t day = remaining_days + 1u;

    uint32_t hour = seconds_of_day / 3600u;
    uint32_t minute = (seconds_of_day % 3600u) / 60u;
    uint32_t second = seconds_of_day % 60u;

    return ((uint64_t)year * 10000000000ULL) + ((uint64_t)month * 100000000ULL) +
           ((uint64_t)day * 1000000ULL) + ((uint64_t)hour * 10000ULL) +
           ((uint64_t)minute * 100ULL) + (uint64_t)second;
}

static void _linearToAddress(uint32_t linear_page, NandFlashAddress* address) {
    uint16_t pages_per_block = self.nand_info.page_count_per_block;

    address->block = (uint16_t)(DATA_BASE_BLOCK + (linear_page / pages_per_block));
    address->page = (uint16_t)(linear_page % pages_per_block);
}

static uint32_t _addressToLinear(const NandFlashAddress* address) {
    if (address->block < DATA_BASE_BLOCK || address->block >= DATA_BASE_BLOCK + DATA_BLOCK_COUNT ||
        address->page >= self.nand_info.page_count_per_block) {
        return UINT32_MAX;
    }
    return ((uint32_t)(address->block - DATA_BASE_BLOCK) * self.nand_info.page_count_per_block) +
           address->page;
}

static void _clearCurrentEntry(void) { memset(&self.current_entry, 0, sizeof(self.current_entry)); }

static bool _hasCurrentEntry(void) {
    return (self.current_entry.magic == EVENT_LOG_MAGIC &&
            self.current_entry.version == EVENT_LOG_VERSION &&
            self.current_entry.record_count > 0 &&
            self.current_entry.record_count <= EVENT_LOG_EDR_MAX_COUNT);
}

static uint32_t _getCurrentAddress(void) {
    if (!_hasCurrentEntry()) {
        return 0;
    }

    uint32_t linear = _addressToLinear(&self.current_entry.end_address);
    if (linear == UINT32_MAX) {
        return _pagesPerDataArea();
    }

    uint32_t next_address = linear + 1u;
    uint32_t total_pages = _pagesPerDataArea();

    return (next_address > total_pages) ? total_pages : next_address;
}

static uint32_t _getCurrentSequence(void) {
    return _hasCurrentEntry() ? self.current_entry.sequence : 0u;
}

static StorageResult _eraseEventDataArea(void) {
    for (uint16_t block = DATA_BASE_BLOCK; block < DATA_BASE_BLOCK + DATA_BLOCK_COUNT; block++) {
        NandFlashAddress address = {
            .block = block,
            .page = 0,
        };

        if (NandFlash_Erase(&address) != APP_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }
    }

    return STORAGE_RESULT_OK;
}

static StorageResult _eraseBufferArea(void) {
    NandFlashAddress address = {.block = BUFFER_BASE_BLOCK, .page = 0};
    return (NandFlash_Erase(&address) == APP_RESULT_OK) ? STORAGE_RESULT_OK : STORAGE_RESULT_ERROR;
}

static StorageResult _eraseMetaArea(void) {
    for (uint16_t block = ENTRY_BASE_BLOCK; block < ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT; block++) {
        NandFlashAddress address = {
            .block = block,
            .page = 0,
        };

        if (NandFlash_Erase(&address) != APP_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }
    }

    return STORAGE_RESULT_OK;
}

static StorageResult _findLastEntry(EventLogEntry* entry) {
    if (entry == NULL) {
        return STORAGE_RESULT_ERROR_INVALID_ARG;
    }

    EventLogEntry latest = {0};
    uint32_t latest_sequence = 0;
    bool found = false;

    for (uint16_t block = ENTRY_BASE_BLOCK; block < ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT; block++) {
        for (uint16_t page = 0; page < self.nand_info.page_count_per_block; page++) {
            NandFlashAddress address = {
                .block = block,
                .page = page,
            };

            EventLogEntry temp;
            if (NandFlash_Read(&address, (uint8_t*)&temp, sizeof(temp)) != APP_RESULT_OK) {
                continue;
            }

            if (temp.magic != EVENT_LOG_MAGIC || temp.version != EVENT_LOG_VERSION) {
                continue;
            }

            uint16_t crc =
                Crc16Modbus_Calculate((uint8_t*)&temp, sizeof(EventLogEntry) - sizeof(temp.crc));
            if (crc != temp.crc) {
                continue;
            }

            if (!found || temp.sequence > latest_sequence) {
                latest = temp;
                latest_sequence = temp.sequence;
                found = true;
            }
        }
    }

    if (!found) {
        return STORAGE_RESULT_ERROR_NOT_FOUND;
    }

    *entry = latest;
    return STORAGE_RESULT_OK;
}

static StorageResult _findEmptyMetaSpace(NandFlashAddress* address) {
    if (address == NULL) {
        return STORAGE_RESULT_ERROR_INVALID_ARG;
    }

    for (uint16_t block = ENTRY_BASE_BLOCK; block < ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT; block++) {
        for (uint16_t page = 0; page < self.nand_info.page_count_per_block; page++) {
            NandFlashAddress target = {
                .block = block,
                .page = page,
            };

            EventLogEntry temp;
            if (NandFlash_Read(&target, (uint8_t*)&temp, sizeof(temp)) != APP_RESULT_OK) {
                continue;
            }

            if (temp.magic == 0xFFFFFFFF) {
                *address = target;
                return STORAGE_RESULT_OK;
            }
        }
    }

    return STORAGE_RESULT_ERROR_STORAGE_FULL;
}

static bool _isValidEntry(const EventLogEntry* entry) {
    if (entry == NULL) {
        return false;
    }

    if (entry->magic != EVENT_LOG_MAGIC || entry->version != EVENT_LOG_VERSION) {
        return false;
    }

    if (entry->record_count == 0 || entry->record_count > EVENT_LOG_EDR_MAX_COUNT) {
        return false;
    }

    uint16_t crc =
        Crc16Modbus_Calculate((uint8_t*)entry, sizeof(EventLogEntry) - sizeof(entry->crc));
    return (crc == entry->crc);
}

static void _insertNewestEntry(EventLogEntry* entries, uint8_t* count, uint8_t max_count,
                               const EventLogEntry* candidate) {
    if (entries == NULL || count == NULL || candidate == NULL || max_count == 0) {
        return;
    }

    if (*count < max_count) {
        entries[*count] = *candidate;
        (*count)++;
    } else {
        if (candidate->sequence <= entries[max_count - 1u].sequence) {
            return;
        }

        entries[max_count - 1u] = *candidate;
    }

    for (uint8_t i = *count; i > 1u; i--) {
        uint8_t curr = (uint8_t)(i - 1u);
        uint8_t prev = (uint8_t)(curr - 1u);

        if (entries[prev].sequence >= entries[curr].sequence) {
            break;
        }

        EventLogEntry temp = entries[prev];
        entries[prev] = entries[curr];
        entries[curr] = temp;
    }
}

static StorageResult _garbageCollectPreservingLatest(uint8_t preserve_limit) {
    if (preserve_limit == 0) {
        return STORAGE_RESULT_OK;
    }

    static EventLogEntry newest_entries[EVENT_LOG_MAX_COUNT];
    memset(newest_entries, 0, sizeof(newest_entries));
    uint8_t newest_count = 0;

    for (uint16_t block = ENTRY_BASE_BLOCK; block < ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT; block++) {
        for (uint16_t page = 0; page < self.nand_info.page_count_per_block; page++) {
            NandFlashAddress address = {
                .block = block,
                .page = page,
            };

            EventLogEntry temp;
            if (NandFlash_Read(&address, (uint8_t*)&temp, sizeof(temp)) != APP_RESULT_OK) {
                continue;
            }

            if (!_isValidEntry(&temp)) {
                continue;
            }

            _insertNewestEntry(newest_entries, &newest_count, preserve_limit, &temp);
        }
    }

    // Reverse newest_entries in-place: newest-first → oldest-first
    for (uint8_t i = 0; i < newest_count / 2u; i++) {
        EventLogEntry tmp = newest_entries[i];
        newest_entries[i] = newest_entries[newest_count - 1u - i];
        newest_entries[newest_count - 1u - i] = tmp;
    }

    if (self.nand_info.page_size > EVENT_LOG_PAGE_BUF_SIZE) {
        return STORAGE_RESULT_ERROR;
    }

    // Step 1: Erase buffer block
    if (_eraseBufferArea() != STORAGE_RESULT_OK) {
        return STORAGE_RESULT_ERROR;
    }

    // Step 2: Copy data area → buffer block, one page at a time
    static uint16_t buf_start_pages[EVENT_LOG_MAX_COUNT];
    static uint16_t buf_page_counts[EVENT_LOG_MAX_COUNT];
    static uint8_t page_buf[EVENT_LOG_PAGE_BUF_SIZE];

    uint8_t valid_count = 0;
    uint16_t buf_write_page = 0;
    uint16_t buf_capacity = self.nand_info.page_count_per_block;

    for (uint8_t i = 0; i < newest_count; i++) {
        EventLogEntry* entry = &newest_entries[i];
        uint32_t total_size = (uint32_t)entry->record_count * sizeof(BmsBriefData);
        uint16_t req_pages = _requiredPageCount(total_size);
        uint32_t base_linear = _addressToLinear(&entry->start_address);

        if (base_linear == UINT32_MAX || buf_write_page + req_pages > buf_capacity) {
            continue;
        }

        uint32_t remaining = total_size;
        uint16_t start_page = buf_write_page;
        bool ok = true;

        for (uint16_t p = 0; p < req_pages; p++) {
            uint32_t src_linear = base_linear + p;
            if (src_linear >= _pagesPerDataArea()) {
                ok = false;
                break;
            }

            NandFlashAddress src_addr, dst_addr;
            _linearToAddress(src_linear, &src_addr);
            dst_addr.block = BUFFER_BASE_BLOCK;
            dst_addr.page = buf_write_page;

            uint16_t rw_size = (remaining > self.nand_info.page_size) ? self.nand_info.page_size
                                                                      : (uint16_t)remaining;

            if (NandFlash_Read(&src_addr, page_buf, rw_size) != APP_RESULT_OK ||
                NandFlash_Write(&dst_addr, page_buf, rw_size) != APP_RESULT_OK) {
                ok = false;
                break;
            }

            buf_write_page++;
            remaining -= rw_size;
        }

        if (!ok) {
            buf_write_page = start_page;  // rollback buffer pointer
            continue;
        }

        if (valid_count != i) {
            newest_entries[valid_count] = *entry;  // compact in-place
        }
        buf_start_pages[valid_count] = start_page;
        buf_page_counts[valid_count] = req_pages;
        valid_count++;
    }

    // Step 3: Erase data area and meta area
    if (_eraseEventDataArea() != STORAGE_RESULT_OK || _eraseMetaArea() != STORAGE_RESULT_OK) {
        _eraseBufferArea();
        return STORAGE_RESULT_ERROR;
    }

    // Step 4: Copy buffer block → data area, write meta entries
    uint32_t write_linear = 0;
    EventLogEntry last_entry = {0};
    bool any_written = false;

    for (uint8_t i = 0; i < valid_count; i++) {
        EventLogEntry* entry = &newest_entries[i];
        uint32_t total_size = (uint32_t)entry->record_count * sizeof(BmsBriefData);
        uint16_t req_pages = buf_page_counts[i];
        uint32_t remaining = total_size;
        bool ok = true;

        for (uint16_t p = 0; p < req_pages; p++) {
            NandFlashAddress src_addr = {.block = BUFFER_BASE_BLOCK,
                                         .page = buf_start_pages[i] + p};
            NandFlashAddress dst_addr;
            _linearToAddress(write_linear + p, &dst_addr);

            uint16_t rw_size = (remaining > self.nand_info.page_size) ? self.nand_info.page_size
                                                                      : (uint16_t)remaining;

            if (NandFlash_Read(&src_addr, page_buf, rw_size) != APP_RESULT_OK ||
                NandFlash_Write(&dst_addr, page_buf, rw_size) != APP_RESULT_OK) {
                ok = false;
                break;
            }

            if (p == 0) entry->start_address = dst_addr;
            if (p == req_pages - 1u) entry->end_address = dst_addr;

            remaining -= rw_size;
        }

        if (!ok) {
            any_written ? (self.current_entry = last_entry) : _clearCurrentEntry();
            _eraseBufferArea();
            return STORAGE_RESULT_ERROR;
        }

        write_linear += req_pages;

        if (_appendEntry(entry) != STORAGE_RESULT_OK) {
            self.current_entry = *entry;
            _eraseBufferArea();
            return STORAGE_RESULT_ERROR;
        }

        last_entry = *entry;
        any_written = true;
    }

    // Step 5: Cleanup buffer block
    _eraseBufferArea();

    any_written ? (self.current_entry = last_entry) : _clearCurrentEntry();
    return STORAGE_RESULT_OK;
}

static StorageResult _appendEntry(EventLogEntry* entry) {
    NandFlashAddress meta_addr;
    StorageResult result = _findEmptyMetaSpace(&meta_addr);

    if (result == STORAGE_RESULT_ERROR_STORAGE_FULL) {
        if (_eraseMetaArea() != STORAGE_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }

        result = _findEmptyMetaSpace(&meta_addr);
    }

    if (result != STORAGE_RESULT_OK) {
        return STORAGE_RESULT_ERROR;
    }

    entry->crc = Crc16Modbus_Calculate((uint8_t*)entry, sizeof(EventLogEntry) - sizeof(entry->crc));

    if (NandFlash_Write(&meta_addr, (uint8_t*)entry, sizeof(EventLogEntry)) != APP_RESULT_OK) {
        return STORAGE_RESULT_ERROR;
    }

    return STORAGE_RESULT_OK;
}

static StorageResult _writeRecordData(const BmsBriefData* data, uint8_t count,
                                      NandFlashAddress* start, NandFlashAddress* end) {
    uint32_t total_size = (uint32_t)count * sizeof(BmsBriefData);
    uint16_t required_pages = _requiredPageCount(total_size);
    uint32_t total_pages = _pagesPerDataArea();

    if (required_pages == 0 || required_pages > total_pages) {
        return STORAGE_RESULT_ERROR_INVALID_ARG;
    }

    uint32_t current_address = _getCurrentAddress();

    if (current_address + required_pages > total_pages) {
        uint8_t preserve_count = EVENT_LOG_MAX_COUNT;
        if (_garbageCollectPreservingLatest(preserve_count) != STORAGE_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }

        current_address = _getCurrentAddress();
        if (current_address + required_pages > total_pages) {
            if (_eraseEventDataArea() != STORAGE_RESULT_OK ||
                _eraseMetaArea() != STORAGE_RESULT_OK) {
                return STORAGE_RESULT_ERROR;
            }

            _clearCurrentEntry();
            current_address = 0;
        }
    }

    uint32_t remaining = total_size;
    const uint8_t* src = (const uint8_t*)data;

    for (uint16_t i = 0; i < required_pages; i++) {
        uint32_t linear_page = current_address + i;
        NandFlashAddress addr;
        _linearToAddress(linear_page, &addr);

        uint16_t write_size =
            (remaining > self.nand_info.page_size) ? self.nand_info.page_size : (uint16_t)remaining;

        if (NandFlash_Write(&addr, src, write_size) != APP_RESULT_OK) {
            return STORAGE_RESULT_ERROR;
        }

        if (i == 0) {
            *start = addr;
        }
        if (i == (required_pages - 1u)) {
            *end = addr;
        }

        src += write_size;
        remaining -= write_size;
    }

    return STORAGE_RESULT_OK;
}

AppResult EventLogStorage_Initialize(uint8_t time_zone_offset_hours) {
    if (!NandFlash_IsInitialized()) {
        if (NandFlash_Initialize() != APP_RESULT_OK) {
            NandFlash_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    self.time_zone_offset_hours = time_zone_offset_hours;

    NandFlashInfo nand_info = {0};
    NandFlash_GetInfo(&nand_info);
    self.nand_info = nand_info;

    _clearCurrentEntry();

    EventLogEntry last_entry;
    StorageResult result = _findLastEntry(&last_entry);

    if (result == STORAGE_RESULT_OK) {
        self.current_entry = last_entry;

    } else if (result != STORAGE_RESULT_ERROR_NOT_FOUND) {
        _clearCurrentEntry();
    }

    return APP_RESULT_OK;
}

AppResult EventLogStorage_Deinitialize() { return APP_RESULT_OK; }

void EventLogStorage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult EventLogStorage_GetInfo(BmsEventLogInfo* info) {
    if (info == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EVENT_LOG_STORAGE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    info->record_unit_size = sizeof(BmsBriefData);
    info->page_size = self.nand_info.page_size;
    info->pages_per_block = self.nand_info.page_count_per_block;
    info->blocks_per_record = 0;
    info->max_record_count = EVENT_LOG_MAX_COUNT;

    return APP_RESULT_OK;
}

AppResult EventLogStorage_GetEntries(BmsEventLogEntry* entries, const uint8_t request_count,
                                     uint8_t* result_count) {
    if (result_count == NULL || (request_count > 0 && entries == NULL)) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EVENT_LOG_STORAGE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    if (request_count == 0) {
        *result_count = 0;
        return APP_RESULT_OK;
    }

    static EventLogEntry newest_entries[EVENT_LOG_MAX_COUNT];
    memset(newest_entries, 0, sizeof(newest_entries));
    uint8_t newest_count = 0;
    uint8_t collect_limit =
        (request_count < EVENT_LOG_MAX_COUNT) ? request_count : EVENT_LOG_MAX_COUNT;

    for (uint16_t block = ENTRY_BASE_BLOCK; block < ENTRY_BASE_BLOCK + ENTRY_BLOCK_COUNT; block++) {
        for (uint16_t page = 0; page < self.nand_info.page_count_per_block; page++) {
            NandFlashAddress address = {
                .block = block,
                .page = page,
            };

            EventLogEntry temp;
            if (NandFlash_Read(&address, (uint8_t*)&temp, sizeof(temp)) != APP_RESULT_OK) {
                continue;
            }

            if (!_isValidEntry(&temp)) {
                continue;
            }

            _insertNewestEntry(newest_entries, &newest_count, collect_limit, &temp);
        }
    }

    for (uint8_t i = 0; i < newest_count; i++) {
        entries[i].datetime_id = newest_entries[i].datetime_id;
        entries[i].type = newest_entries[i].type;
        entries[i].sequence = newest_entries[i].sequence;
        entries[i].record_count = newest_entries[i].record_count;
        entries[i].start_address.block = newest_entries[i].start_address.block;
        entries[i].start_address.page = newest_entries[i].start_address.page;
        entries[i].end_address.block = newest_entries[i].end_address.block;
        entries[i].end_address.page = newest_entries[i].end_address.page;
        entries[i].reset_flag = newest_entries[i].reset_flag;
        entries[i].protection.flag = newest_entries[i].protection_flag;
        entries[i].protection.snapshot_value = newest_entries[i].protection_snapshot_value;
        entries[i].protection.snapshot_threshold = newest_entries[i].protection_snapshot_threshold;
    }

    *result_count = newest_count;
    return APP_RESULT_OK;
}

AppResult EventLogStorage_Record(BmsEventLogType type, uint32_t reset_flag,
                                 uint32_t protection_flag, int32_t protection_snapshot_value,
                                 int32_t protection_snapshot_threshold, const BmsBriefData* data,
                                 uint8_t count) {
    if (data == NULL || count == 0 || count > EVENT_LOG_EDR_MAX_COUNT) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EVENT_LOG_STORAGE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    EventLogEntry entry = {
        .magic = EVENT_LOG_MAGIC,
        .datetime_id =
            _toDateTimeIdFromTimestamp(data[count - 1u].timestamp, self.time_zone_offset_hours),
        .type = type,
        .reset_flag = reset_flag,
        .protection_flag = protection_flag,
        .protection_snapshot_value = protection_snapshot_value,
        .protection_snapshot_threshold = protection_snapshot_threshold,
        .record_count = count,
        .version = EVENT_LOG_VERSION,
        .sequence = _getCurrentSequence() + 1u,
    };

    StorageResult result = _writeRecordData(data, count, &entry.start_address, &entry.end_address);
    if (result != STORAGE_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EVENT_LOG_STORAGE_DRV_WRITE);
        return APP_RESULT_FAIL;
    }

    if (_appendEntry(&entry) != STORAGE_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EVENT_LOG_STORAGE_DRV_WRITE);
        return APP_RESULT_FAIL;
    }

    self.current_entry = entry;

    return APP_RESULT_OK;
}

AppResult EventLogStorage_EraseAllRecords(void) {
    if (_eraseEventDataArea() != STORAGE_RESULT_OK || _eraseMetaArea() != STORAGE_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EVENT_LOG_STORAGE_DRV_ERASE);
        return APP_RESULT_FAIL;
    }

    _clearCurrentEntry();

    return APP_RESULT_OK;
}

AppResult EventLogStorage_ReadRawData(uint8_t* data, uint16_t size, uint16_t block, uint16_t page) {
    if (data == NULL || size == 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EVENT_LOG_STORAGE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    NandFlashAddress address = {
        .block = block,
        .page = page,
    };

    if (NandFlash_Read(&address, data, size) != APP_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_EVENT_LOG_STORAGE_DRV_READ);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}
