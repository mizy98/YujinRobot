#include "event_log_storage.h"

#include <stddef.h>

WEAK_LINK AppResult EventLogStorage_Initialize(uint8_t time_zone_offset_hours) {
    (void)time_zone_offset_hours;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult EventLogStorage_Deinitialize() { return APP_RESULT_FAIL; }

WEAK_LINK void EventLogStorage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        APP_ERROR_SET_INFO(*info, APP_ERROR_NOT_IMPLEMENT);
    }
}

WEAK_LINK AppResult EventLogStorage_GetInfo(BmsEventLogInfo* info) {
    (void)info;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult EventLogStorage_GetEntries(BmsEventLogEntry* entries,
                                               const uint8_t request_count, uint8_t* result_count) {
    (void)entries;
    (void)request_count;
    (void)result_count;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult EventLogStorage_Record(BmsEventLogType type, uint32_t reset_flag,
                                           uint32_t protection_flag,
                                           int32_t protection_snapshot_value,
                                           int32_t protection_snapshot_threshold,
                                           const BmsBriefData* data, uint8_t count) {
    (void)type;
    (void)reset_flag;
    (void)protection_flag;
    (void)protection_snapshot_value;
    (void)protection_snapshot_threshold;
    (void)data;
    (void)count;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult EventLogStorage_EraseAllRecords(void) { return APP_RESULT_FAIL; }

WEAK_LINK AppResult EventLogStorage_ReadRawData(uint8_t* data, uint16_t size, uint16_t block,
                                                uint16_t page) {
    (void)data;
    (void)size;
    (void)block;
    (void)page;
    return APP_RESULT_FAIL;
}