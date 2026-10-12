#include "data_log_storage.h"

WEAK_LINK AppResult DataLogStorage_Initialize(const DataLogStorageCallback* callback,
                                              uint32_t timestamp, uint8_t time_zone_offset_hours) {
    (void)callback;
    (void)timestamp;
    (void)time_zone_offset_hours;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult DataLogStorage_Deinitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void DataLogStorage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        APP_ERROR_SET_INFO(*info, APP_ERROR_NOT_IMPLEMENT);
    }
}

WEAK_LINK AppResult DataLogStorage_GetInfo(BmsDataLogInfo* info) {
    (void)info;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult DataLogStorage_GetCurrentEntry(BmsDataLogEntry* entry) {
    (void)entry;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult DataLogStorage_GetClosedEntries(BmsDataLogEntry* meta_datas,
                                                    const uint8_t request_count, uint8_t* count) {
    (void)meta_datas;
    (void)request_count;
    if (count != NULL) {
        *count = 0;
    }
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult DataLogStorage_FindClosedEntryByStartAddress(uint16_t block, uint16_t page,
                                                                 BmsDataLogEntry* entry) {
    (void)block;
    (void)page;
    (void)entry;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult DataLogStorage_Record(const BmsBriefData* data, float* usage) {
    (void)data;
    (void)usage;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult DataLogStorage_EraseAllRecords(void) { return APP_RESULT_FAIL; }

WEAK_LINK AppResult DataLogStorage_StartReadStream(const BmsDataLogEntry* meta_data) {
    (void)meta_data;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult DataLogStorage_ReadRawData(uint8_t* data, uint16_t size, uint16_t block,
                                               uint16_t page) {
    (void)block;
    (void)page;
    return APP_RESULT_FAIL;
}