#include "black_box.h"

#include "../interface/data_log_storage.h"
#include "../interface/error_log_storage.h"
#include "../interface/event_log_storage.h"
#include "../interface/system.h"

#define self (black_box_private)

#define MAX_CONCURRENT_EVENTS (5u)

typedef struct {
    bool is_active;
    uint8_t after_count;
    BmsEventLogType type;
    uint32_t reset_flag;
    uint32_t protection_flag;
    int32_t protection_snapshot_value;
    int32_t protection_snapshot_threshold;
} EventCollector;

#pragma pack(push, 1)
static struct {
    AppErrorInfo error_info;

    BmsData last_data;

    // EDR: Event Data Recorder
    BmsBriefData edr_buffer[(BLACK_BOX_EVENT_MARGIN_SAMPLE_COUNT * 2) + 1];
    uint8_t edr_write_index;
    bool edr_buffer_full;

    uint32_t last_protection_flag;

    EventCollector event_collectors[MAX_CONCURRENT_EVENTS];

    BmsPackState last_pack_state;
    AppResetReason reset_reason;
    uint32_t reset_flag;

    bool is_first_record;

    BlackBoxCallback callback;
} black_box_private;
#pragma pack(pop)

BmsBriefData toBreifData(const BmsData* data) {
    BmsBriefData brief_data;

    brief_data.timestamp = data->timestamp;
    brief_data.real_soc = data->soc.real_soc;
    brief_data.display_soc = data->soc.display_soc;
    brief_data.pack_state = data->pack_state;

    for (uint8_t i = 0; i < VOLTAGE_MAX_COUNT; i++) {
        brief_data.bank_voltages[i] = data->voltage.bank_voltages[i].voltage;
    }
    brief_data.current = data->current.value;
    for (uint8_t i = 0; i < TEMPERATURE_MAX_PACK_COUNT; i++) {
        brief_data.pack_temperatures[i] = data->pack_temperature.temperatures[i];
    }

    brief_data.protection_state = data->protection_state;
    brief_data.warning_state = data->warning_state;
    brief_data.main_contactor_state = data->main_contactor_state;
    brief_data.charge_contactor_state = data->charge_contactor_state;
    brief_data.discharge_contactor_state = data->discharge_contactor_state;
    brief_data.heater_state = data->heater_state;
    brief_data.cooler_state = data->cooler_state;

    brief_data.current_offset_voltage = data->current.adc_offset_voltage;

    return brief_data;
}

static uint8_t _afterCountForEventType(BmsEventLogType type) {
    if (type <= BMS_EVENT_LOG_TYPE_BOOT_UNKNOWN_RESET) {
        return BLACK_BOX_EVENT_MARGIN_SAMPLE_COUNT;
    }
    return (uint8_t)(BLACK_BOX_EVENT_MARGIN_SAMPLE_COUNT * 2u);
}

static BmsEventLogType _parseEventType(const BmsBriefData* data, int32_t protection_snapshot_value,
                                       int32_t protection_snapshot_threshold,
                                       EventCollector* collector) {
    BmsEventLogType type = BMS_EVENT_LOG_TYPE_NONE;
    uint32_t protection_flag = 0;
    int32_t snapshot_value = 0;
    int32_t snapshot_threshold = 0;

    if (self.is_first_record) {
        self.is_first_record = false;

        switch (self.reset_reason) {
            case APP_RESET_REASON_POWER_ON_RESET:
                type = BMS_EVENT_LOG_TYPE_BOOT_POWER_ON;
                break;
            case APP_RESET_REASON_PIN_RESET:
                type = BMS_EVENT_LOG_TYPE_BOOT_PIN_RESET;
                break;
            case APP_RESET_REASON_SOFTWARE_RESET:
                type = BMS_EVENT_LOG_TYPE_BOOT_SOFTWARE_RESET;
                break;
            case APP_RESET_REASON_INDEPENDENT_WATCHDOG_RESET:
                type = BMS_EVENT_LOG_TYPE_BOOT_INDEPENDENT_WATCHDOG_RESET;
                break;
            case APP_RESET_REASON_WINDOW_WATCHDOG_RESET:
                type = BMS_EVENT_LOG_TYPE_BOOT_WINDOW_WATCHDOG_RESET;
                break;
            case APP_RESET_REASON_LOW_POWER_RESET:
                type = BMS_EVENT_LOG_TYPE_BOOT_LOW_POWER_RESET;
                break;
            case APP_RESET_REASON_BROWN_OUT_RESET:
                type = BMS_EVENT_LOG_TYPE_BOOT_BROWN_OUT_RESET;
                break;
            default:
                type = BMS_EVENT_LOG_TYPE_BOOT_UNKNOWN_RESET;
                break;
        }
    } else if (self.last_protection_flag != data->protection_state.value &&
               data->protection_state.value != 0) {
        protection_flag = data->protection_state.value;
        self.last_protection_flag = data->protection_state.value;

        snapshot_value = protection_snapshot_value;
        snapshot_threshold = protection_snapshot_threshold;

        type = BMS_EVENT_LOG_TYPE_PROTECTION_OCCURRED;

    } else if (self.last_protection_flag != data->protection_state.value &&
               data->protection_state.value == 0) {
        protection_flag = self.last_protection_flag;
        self.last_protection_flag = data->protection_state.value;

        snapshot_value = protection_snapshot_value;
        snapshot_threshold = protection_snapshot_threshold;

        type = BMS_EVENT_LOG_TYPE_PROTECTION_RELEASED;

    } else if (self.last_pack_state != data->pack_state &&
               data->pack_state == BMS_PACK_STATE_SLEEP) {
        self.last_pack_state = data->pack_state;

        type = BMS_EVENT_LOG_TYPE_SLEEP_ENTER;

    } else if (self.last_pack_state != data->pack_state &&
               self.last_pack_state == BMS_PACK_STATE_SLEEP) {
        self.last_pack_state = data->pack_state;

        type = BMS_EVENT_LOG_TYPE_SLEEP_EXIT;
    }

    if (type != BMS_EVENT_LOG_TYPE_NONE && collector != NULL) {
        collector->is_active = true;
        collector->type = type;
        collector->after_count = _afterCountForEventType(type);
        collector->reset_flag = self.reset_flag;
        collector->protection_flag = protection_flag;
        collector->protection_snapshot_value = snapshot_value;
        collector->protection_snapshot_threshold = snapshot_threshold;
    }

    return type;
}

static uint8_t _sortEdr(BmsBriefData* output) {
    if (output == NULL) {
        return 0;
    }

    const uint8_t buffer_size = (uint8_t)ARRAY_COUNT(self.edr_buffer);
    const uint8_t valid_count = self.edr_buffer_full ? buffer_size : self.edr_write_index;

    if (valid_count == 0) {
        return 0;
    }

    const uint8_t start_index = self.edr_buffer_full ? self.edr_write_index : 0;

    for (uint8_t i = 0; i < valid_count; i++) {
        uint8_t src_index = (uint8_t)((start_index + i) % buffer_size);
        output[i] = self.edr_buffer[src_index];
    }

    return valid_count;
}

AppResult BlackBox_Initialize(BlackBoxCallback* callback, uint32_t timestamp,
                              uint8_t time_zone_offset_hours) {
    if (DataLogStorage_Initialize(
            &(DataLogStorageCallback){
                .onRecordStored = callback->onRecordStored,
                .onRecordLoaded = callback->onRecordLoaded,
                .onRecordLoadFinished = callback->onRecordLoadFinished,
            },
            timestamp, time_zone_offset_hours) != APP_RESULT_OK) {
        DataLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (EventLogStorage_Initialize(time_zone_offset_hours) != APP_RESULT_OK) {
        EventLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (ErrorLogStorage_Initialize() != APP_RESULT_OK) {
        ErrorLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    AppResetReason reset_reason;
    uint32_t raw_reset_flag;

    System_GetResetReason(&reset_reason, &raw_reset_flag);

    self.callback = *callback;

    self.reset_reason = reset_reason;
    self.reset_flag = raw_reset_flag;

    self.is_first_record = true;

    self.last_protection_flag = 0;
    self.last_pack_state = BMS_PACK_STATE_IDLE;

    for (uint8_t i = 0; i < MAX_CONCURRENT_EVENTS; i++) {
        self.event_collectors[i].is_active = false;
        self.event_collectors[i].after_count = 0;
        self.event_collectors[i].type = BMS_EVENT_LOG_TYPE_NONE;
    }

    return APP_RESULT_OK;
}

AppResult BlackBox_Deinitialize() {
    if (DataLogStorage_Deinitialize() != APP_RESULT_OK) {
        DataLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (EventLogStorage_Deinitialize() != APP_RESULT_OK) {
        EventLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (ErrorLogStorage_Deinitialize() != APP_RESULT_OK) {
        ErrorLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void BlackBox_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult BlackBox_RecordData(const BmsData* data, float* usage, int32_t protection_snapshot_value,
                              int32_t protection_snapshot_threshold) {
    if (data == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BLACK_BOX_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    // TODO: enable this snippet after test
    // if (self.last_data.timestamp == data->timestamp) {
    //     // same data, skip recording
    //     return APP_RESULT_OK;
    // }

    BmsBriefData brief_data = toBreifData(data);

    self.edr_buffer[self.edr_write_index] = brief_data;
    self.edr_write_index =
        (self.edr_write_index + 1) % ((BLACK_BOX_EVENT_MARGIN_SAMPLE_COUNT * 2) + 1);
    if (!self.edr_buffer_full && self.edr_write_index == 0) {
        self.edr_buffer_full = true;
    }

    if (DataLogStorage_Record(&brief_data, usage) != APP_RESULT_OK) {
        DataLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    EventCollector* free_slot = NULL;
    for (uint8_t j = 0; j < MAX_CONCURRENT_EVENTS; j++) {
        if (!self.event_collectors[j].is_active) {
            free_slot = &self.event_collectors[j];
            break;
        }
    }
    _parseEventType(&brief_data, protection_snapshot_value, protection_snapshot_threshold,
                    free_slot);

    for (uint8_t i = 0; i < MAX_CONCURRENT_EVENTS; i++) {
        if (!self.event_collectors[i].is_active) {
            continue;
        }

        if (self.event_collectors[i].after_count > 0) {
            self.event_collectors[i].after_count--;
            continue;
        }

        self.event_collectors[i].is_active = false;

        BmsBriefData ordered_edr_buffer[ARRAY_COUNT(self.edr_buffer)] = {0};
        uint8_t ordered_count = _sortEdr(ordered_edr_buffer);

        if (ordered_count == 0) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BLACK_BOX_INVALID_ARG);
            return APP_RESULT_FAIL;
        }

        if (EventLogStorage_Record(self.event_collectors[i].type,
                                   self.event_collectors[i].reset_flag,
                                   self.event_collectors[i].protection_flag,
                                   self.event_collectors[i].protection_snapshot_value,
                                   self.event_collectors[i].protection_snapshot_threshold,
                                   ordered_edr_buffer, ordered_count) != APP_RESULT_OK) {
            EventLogStorage_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }

        self.reset_flag = 0;

        if (self.callback.onEventRecorded != NULL) {
            self.callback.onEventRecorded(self.event_collectors[i].type);
        }
    }

    return APP_RESULT_OK;
}

AppResult BlackBox_GetDataLogInfo(BmsDataLogInfo* info) {
    if (info == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BLACK_BOX_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    if (DataLogStorage_GetInfo(info) != APP_RESULT_OK) {
        DataLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult BlackBox_GetCurrentDataLogEntry(BmsDataLogEntry* entry) {
    if (entry == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BLACK_BOX_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    if (DataLogStorage_GetCurrentEntry(entry) != APP_RESULT_OK) {
        DataLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult BlackBox_GetDataLogEntryHistory(BmsDataLogEntry* entries, const uint8_t request_count,
                                          uint8_t* result_count) {
    return DataLogStorage_GetClosedEntries(entries, request_count, result_count);
}

AppResult BlackBox_ReadDataLog(uint8_t* data, uint16_t size, uint16_t block, uint16_t page) {
    if (DataLogStorage_ReadRawData(data, size, block, page) != APP_RESULT_OK) {
        DataLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult BlackBox_StartDataLogReadStream(const BmsDataLogEntry* meta_data) {
    if (meta_data == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BLACK_BOX_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    if (DataLogStorage_StartReadStream(meta_data) != APP_RESULT_OK) {
        DataLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult BlackBox_StartDataLogReadStreamByAddress(uint16_t block, uint16_t page) {
    BmsDataLogEntry entry;

    if (DataLogStorage_FindClosedEntryByStartAddress(block, page, &entry) != APP_RESULT_OK) {
        DataLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (DataLogStorage_StartReadStream(&entry) != APP_RESULT_OK) {
        DataLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult BlackBox_EraseAllDataLogs(void) {
    if (DataLogStorage_EraseAllRecords() != APP_RESULT_OK) {
        DataLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult BlackBox_GetEventLogInfo(BmsEventLogInfo* info) {
    if (info == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BLACK_BOX_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    if (EventLogStorage_GetInfo(info) != APP_RESULT_OK) {
        EventLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult BlackBox_GetEventLogEntryHistory(BmsEventLogEntry* entries, const uint8_t request_count,
                                           uint8_t* result_count) {
    return EventLogStorage_GetEntries(entries, request_count, result_count);
}

AppResult BlackBox_ReadEventLog(uint8_t* data, uint16_t size, uint16_t block, uint16_t page) {
    return EventLogStorage_ReadRawData(data, size, block, page);
}

AppResult BlackBox_EraseAllEventLogs(void) { return EventLogStorage_EraseAllRecords(); }

AppResult BlackBox_RecordError(const AppErrorInfo* error_info) {
    if (ErrorLogStorage_Record(error_info) != APP_RESULT_OK) {
        ErrorLogStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}