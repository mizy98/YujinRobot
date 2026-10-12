#ifndef BLACK_BOX_H
#define BLACK_BOX_H

#define BLACK_BOX_EVENT_MARGIN_SAMPLE_COUNT (5u)

#include "../model/app_model.h"

typedef struct {
    void (*onRecordStored)(const BmsBriefData* data, uint16_t count);
    void (*onRecordLoaded)(const BmsBriefData* data, uint16_t count);
    void (*onRecordLoadFinished)(void);
    void (*onEventRecorded)(BmsEventLogType event_type);
} BlackBoxCallback;

AppResult BlackBox_Initialize(BlackBoxCallback* callback, uint32_t timestamp,
                              uint8_t time_zone_offset_hours);
AppResult BlackBox_Deinitialize(void);
void BlackBox_GetLastErrorInfo(AppErrorInfo* info);

// data log + event log
AppResult BlackBox_RecordData(const BmsData* data, float* usage, int32_t protection_snapshot_value,
                              int32_t protection_snapshot_threshold);

// data log
AppResult BlackBox_GetDataLogInfo(BmsDataLogInfo* info);
AppResult BlackBox_GetCurrentDataLogEntry(BmsDataLogEntry* entry);
AppResult BlackBox_GetDataLogEntryHistory(BmsDataLogEntry* entries, const uint8_t request_count,
                                          uint8_t* result_count);
AppResult BlackBox_ReadDataLog(uint8_t* data, uint16_t size, uint16_t block, uint16_t page);
AppResult BlackBox_StartDataLogReadStream(const BmsDataLogEntry* meta_data);
AppResult BlackBox_StartDataLogReadStreamByAddress(uint16_t block, uint16_t page);
AppResult BlackBox_EraseAllDataLogs(void);

// event log
AppResult BlackBox_GetEventLogInfo(BmsEventLogInfo* info);
AppResult BlackBox_GetEventLogEntryHistory(BmsEventLogEntry* entries, const uint8_t request_count,
                                           uint8_t* result_count);
AppResult BlackBox_ReadEventLog(uint8_t* data, uint16_t size, uint16_t block, uint16_t page);
AppResult BlackBox_EraseAllEventLogs(void);

// error
AppResult BlackBox_RecordError(const AppErrorInfo* error_info);

#endif