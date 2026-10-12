#ifndef EVENT_LOG_STORAGE_H
#define EVENT_LOG_STORAGE_H

#include "../model/app_model.h"

AppResult EventLogStorage_Initialize(uint8_t time_zone_offset_hours);
AppResult EventLogStorage_Deinitialize();
void EventLogStorage_GetLastErrorInfo(AppErrorInfo* info);

AppResult EventLogStorage_GetInfo(BmsEventLogInfo* info);
AppResult EventLogStorage_GetEntries(BmsEventLogEntry* entries, const uint8_t request_count,
                                     uint8_t* result_count);
AppResult EventLogStorage_Record(BmsEventLogType type, uint32_t reset_flag,
                                 uint32_t protection_flag, int32_t protection_snapshot_value,
                                 int32_t protection_snapshot_threshold, const BmsBriefData* data,
                                 uint8_t count);
AppResult EventLogStorage_EraseAllRecords(void);
AppResult EventLogStorage_ReadRawData(uint8_t* data, uint16_t size, uint16_t block, uint16_t page);

#endif