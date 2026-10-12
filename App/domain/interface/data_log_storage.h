#ifndef DATA_LOG_STORAGE_H
#define DATA_LOG_STORAGE_H

#include <stddef.h>

#include "../model/app_model.h"

#pragma pack(push, 1)
typedef struct {
    void (*onRecordStored)(const BmsBriefData* data, uint16_t count);
    void (*onRecordLoaded)(const BmsBriefData* data, uint16_t count);
    void (*onRecordLoadFinished)(void);
} DataLogStorageCallback;
#pragma pack(pop)

AppResult DataLogStorage_Initialize(const DataLogStorageCallback* callback, uint32_t timestamp,
                                    uint8_t time_zone_offset_hours);
AppResult DataLogStorage_Deinitialize(void);
void DataLogStorage_GetLastErrorInfo(AppErrorInfo* info);

AppResult DataLogStorage_GetInfo(BmsDataLogInfo* info);
AppResult DataLogStorage_GetCurrentEntry(BmsDataLogEntry* entry);
AppResult DataLogStorage_GetClosedEntries(BmsDataLogEntry* meta_datas, const uint8_t request_count,
                                          uint8_t* count);
AppResult DataLogStorage_FindClosedEntryByStartAddress(uint16_t block, uint16_t page,
                                                       BmsDataLogEntry* entry);
AppResult DataLogStorage_Record(const BmsBriefData* data, float* usage);
AppResult DataLogStorage_EraseAllRecords(void);
AppResult DataLogStorage_StartReadStream(const BmsDataLogEntry* meta_data);
AppResult DataLogStorage_ReadRawData(uint8_t* data, uint16_t size, uint16_t block, uint16_t page);

#endif