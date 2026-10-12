#include "ui_comm_handler.h"

#include "../../domain/interface/system.h"
#include "../../domain/interface/system_time.h"
#include "../../domain/service/validator.h"

#define self (ui_comm_handler_private)

#define QUEUE_TIMEOUT (1000u)

static struct {
    AppErrorInfo error_info;
} ui_comm_handler_private;

static Rtos* _rtos;

static void _setConfig(const BmsConfig* config) {
    AppErrorInfo error_info = {0};

    if (config != NULL && Validator_ValidateBmsConfig(config, &error_info) == APP_RESULT_OK) {
        SystemTime_SetTimestamp((uint64_t)config->meta.timestamp);

        _rtos->app->config = *config;

        Rtos_SetEvent(_rtos->event.storage, EVENT_STORAGE_UPDATE_CONFIG);
        Rtos_SetEvent(_rtos->event.protection, EVENT_PROTECTION_UPDATE);
        Rtos_SetEvent(_rtos->event.temperature, EVENT_TEMPERATURE_UPDATE_CONFIG);
    }
}

static AppResult _getDataLogInfo(BmsDataLogInfo* info) {
    if (info == NULL) {
        return APP_RESULT_FAIL;
    }

    while (Rtos_ReceiveQueue(_rtos->queue.log_info, info, 0u) == APP_RESULT_OK) {
    }

    Rtos_SetEvent(_rtos->event.storage, EVENT_STORAGE_LOAD_DATA_LOG_INFO);

    BmsDataLogInfo temp = {0};
    if (Rtos_ReceiveQueue(_rtos->queue.log_info, &temp, QUEUE_TIMEOUT) != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    *info = temp;
    return APP_RESULT_OK;
}

static AppResult _getDataLogRecord(uint16_t block, uint16_t page, uint8_t* buffer,
                                   uint16_t buffer_size) {
    while (Rtos_ReceiveQueue(_rtos->queue.log_data, buffer, 0u) == APP_RESULT_OK) {
    }

    _rtos->task.storage->data_log_request.block = block;
    _rtos->task.storage->data_log_request.page = page;
    Rtos_SetEvent(_rtos->event.storage, EVENT_STORAGE_LOAD_DATA_LOG);

    if (Rtos_ReceiveQueue(_rtos->queue.log_data, buffer, QUEUE_TIMEOUT) != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

static void _getDataLogEntry(BmsDataLogEntry* entries, const uint8_t request_count,
                             uint8_t* result_count) {
    if (entries != NULL && result_count != NULL) {
        *result_count = 0;

        uint8_t requested_count = request_count;
        if (requested_count > DATA_LOG_MAX_COUNT) {
            requested_count = DATA_LOG_MAX_COUNT;
        }

        if (requested_count == 0u) {
            return;
        }

        BmsDataLogEntry dummy;
        while (Rtos_ReceiveQueue(_rtos->queue.data_log_entry, &dummy, 0u) == APP_RESULT_OK) {
        }

        _rtos->task.storage->data_log_request.entry_count = requested_count;
        Rtos_SetEvent(_rtos->event.storage, EVENT_STORAGE_LOAD_DATA_LOG_ENTRY);

        for (uint8_t i = 0; i < requested_count; i++) {
            if (Rtos_ReceiveQueue(_rtos->queue.data_log_entry, &entries[i], QUEUE_TIMEOUT) !=
                APP_RESULT_OK) {
                break;
            }
            (*result_count)++;
        }
    }

    if (*result_count == 0) {
    }
}

static void _getEventLogInfo(BmsEventLogInfo* info) {
    if (info == NULL) {
        return;
    }

    while (Rtos_ReceiveQueue(_rtos->queue.log_info, info, 0u) == APP_RESULT_OK) {
    }

    Rtos_SetEvent(_rtos->event.storage, EVENT_STORAGE_LOAD_EVENT_LOG_INFO);

    BmsEventLogInfo temp = {0};
    if (Rtos_ReceiveQueue(_rtos->queue.log_info, &temp, QUEUE_TIMEOUT) != APP_RESULT_OK) {
        return;
    }

    *info = temp;
}

static void _getEventLogEntry(BmsEventLogEntry* entries, const uint8_t request_count,
                              uint8_t* result_count) {
    if (entries == NULL || result_count == NULL) {
        return;
    }

    *result_count = 0;

    uint8_t requested_count = request_count;
    if (requested_count > EVENT_LOG_MAX_COUNT) {
        requested_count = EVENT_LOG_MAX_COUNT;
    }

    if (requested_count == 0u) {
        return;
    }

    BmsEventLogEntry dummy;
    while (Rtos_ReceiveQueue(_rtos->queue.event_log_entry, &dummy, 0u) == APP_RESULT_OK) {
    }

    _rtos->task.storage->event_log_request.entry_count = requested_count;
    Rtos_SetEvent(_rtos->event.storage, EVENT_STORAGE_LOAD_EVENT_LOG_ENTRY);

    for (uint8_t i = 0; i < requested_count; i++) {
        if (Rtos_ReceiveQueue(_rtos->queue.event_log_entry, &entries[i], QUEUE_TIMEOUT) !=
            APP_RESULT_OK) {
            break;
        }
        (*result_count)++;
    }
}

static AppResult _getEventLogRecord(uint16_t block, uint16_t page, uint8_t* buffer,
                                    uint16_t buffer_size) {
    while (Rtos_ReceiveQueue(_rtos->queue.log_data, buffer, 0u) == APP_RESULT_OK) {
    }

    _rtos->task.storage->event_log_request.block = block;
    _rtos->task.storage->event_log_request.page = page;
    Rtos_SetEvent(_rtos->event.storage, EVENT_STORAGE_LOAD_EVENT_LOG);

    if (Rtos_ReceiveQueue(_rtos->queue.log_data, buffer, QUEUE_TIMEOUT) != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult UiCommHandler_Initialize(Rtos* rtos) {
    if (rtos == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_UI_COMM_HANDLER_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    _rtos = rtos;

    return APP_RESULT_OK;
}

void UiCommHandler_Deinitialize() {}

void UiCommHandler_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

void UiCommHandler_Process(UiProtocolCommand command, uint8_t sequence, uint8_t* payload,
                           uint16_t length) {
    switch (command) {
        case UI_PROTOCOL_RESET: {
            System_Reset();
            break;
        }

        case UI_PROTOCOL_GET_SIZE_INFO: {
            BmsInfo info = _rtos->app->info;
            if (UiProtocol_SendDataWithSequence(UI_PROTOCOL_GET_SIZE_INFO, sequence,
                                                (const uint8_t*)&info, 8u) != APP_RESULT_OK) {
                UiProtocol_GetLastErrorInfo(&self.error_info);
            }
            break;
        }

        case UI_PROTOCOL_START_DATA_STREAM:
        case UI_PROTOCOL_STOP_DATA_STREAM:
        case UI_PROTOCOL_START_BRIEF_DATA_STREAM:
        case UI_PROTOCOL_STOP_BRIEF_DATA_STREAM: {
            // TODO: implement if needed
            break;
        }

        case UI_PROTOCOL_GET_INFO: {
            BmsInfo info = _rtos->app->info;
            if (UiProtocol_SendDataWithSequence(UI_PROTOCOL_GET_INFO, sequence,
                                                (const uint8_t*)&info,
                                                sizeof(info)) != APP_RESULT_OK) {
                UiProtocol_GetLastErrorInfo(&self.error_info);
            }
            break;
        }

        case UI_PROTOCOL_SET_CONFIG: {
            if (length >= sizeof(BmsConfig)) {
                _setConfig((const BmsConfig*)payload);
            }
            break;
        }

        case UI_PROTOCOL_GET_CONFIG: {
            BmsConfig config = _rtos->app->config;
            if (UiProtocol_SendDataWithSequence(UI_PROTOCOL_GET_CONFIG, sequence,
                                                (const uint8_t*)&config,
                                                sizeof(config)) != APP_RESULT_OK) {
                UiProtocol_GetLastErrorInfo(&self.error_info);
            }
            break;
        }

        case UI_PROTOCOL_GET_DATA: {
            BmsData data = _rtos->app->data;
            if (UiProtocol_SendDataWithSequence(UI_PROTOCOL_GET_DATA, sequence,
                                                (const uint8_t*)&data,
                                                sizeof(data)) != APP_RESULT_OK) {
                UiProtocol_GetLastErrorInfo(&self.error_info);
            }
            break;
        }

        case UI_PROTOCOL_GET_RTOS_STATUS: {
            break;
        }

        case UI_PROTOCOL_GET_DATA_LOG_ENTRY: {
            uint8_t requested_count = length > 0u ? payload[0] : 0u;
            BmsDataLogEntry meta_data[DATA_LOG_MAX_COUNT];
            uint8_t count = 0u;

            _getDataLogEntry(meta_data, requested_count, &count);

            (void)UiProtocol_SendDataWithSequence(UI_PROTOCOL_GET_DATA_LOG_ENTRY, sequence,
                                                  (const uint8_t*)meta_data,
                                                  (uint16_t)(count * sizeof(BmsDataLogEntry)));
            break;
        }

        case UI_PROTOCOL_GET_DATA_LOG_INFO: {
            BmsDataLogInfo info = {0};
            _getDataLogInfo(&info);
            (void)UiProtocol_SendDataWithSequence(UI_PROTOCOL_GET_DATA_LOG_INFO, sequence,
                                                  (const uint8_t*)&info, sizeof(info));
            break;
        }

        case UI_PROTOCOL_GET_DATA_LOG_CHUNK: {
            if (length < 4) {
                break;
            }

            uint16_t block = *((const uint16_t*)payload);
            uint16_t page = *((const uint16_t*)(payload + 2));
            uint8_t buffer[DATA_LOG_MAX_PAGE_SIZE] = {0};

            if (_getDataLogRecord(block, page, buffer, sizeof(buffer)) != APP_RESULT_OK) {
                break;
            }

            (void)UiProtocol_SendDataWithSequence(UI_PROTOCOL_GET_DATA_LOG_CHUNK, sequence, buffer,
                                                  sizeof(buffer));
            break;
        }

        case UI_PROTOCOL_RESET_DATA_LOG: {
            Rtos_SetEvent(_rtos->event.storage, EVENT_STORAGE_RESET_DATA_LOG);
            break;
        }

        case UI_PROTOCOL_GET_EVENT_LOG_ENTRY: {
            uint8_t requested_count = length > 0u ? payload[0] : 0u;
            BmsEventLogEntry entries[EVENT_LOG_MAX_COUNT];
            uint8_t count = 0u;

            _getEventLogEntry(entries, requested_count, &count);

            (void)UiProtocol_SendDataWithSequence(UI_PROTOCOL_GET_EVENT_LOG_ENTRY, sequence,
                                                  (const uint8_t*)entries,
                                                  (uint16_t)(count * sizeof(BmsEventLogEntry)));
            break;
        }

        case UI_PROTOCOL_GET_EVENT_LOG_INFO: {
            BmsEventLogInfo info = {0};

            _getEventLogInfo(&info);

            (void)UiProtocol_SendDataWithSequence(UI_PROTOCOL_GET_EVENT_LOG_INFO, sequence,
                                                  (const uint8_t*)&info, sizeof(info));
            break;
        }

        case UI_PROTOCOL_GET_EVENT_LOG_CHUNK: {
            if (length < 4) {
                break;
            }

            uint16_t block = *((const uint16_t*)payload);
            uint16_t page = *((const uint16_t*)(payload + 2));
            uint8_t buffer[DATA_LOG_MAX_PAGE_SIZE] = {0};

            if (_getEventLogRecord(block, page, buffer, sizeof(buffer)) != APP_RESULT_OK) {
                break;
            }

            (void)UiProtocol_SendDataWithSequence(UI_PROTOCOL_GET_EVENT_LOG_CHUNK, sequence, buffer,
                                                  sizeof(buffer));
            break;
        }

        case UI_PROTOCOL_RESET_EVENT_LOG: {
            Rtos_SetEvent(_rtos->event.storage, EVENT_STORAGE_RESET_EVENT_LOG);
            break;
        }

        case UI_PROTOCOL_SET_TIMESTAMP: {
            if (length < sizeof(uint32_t)) {
                break;
            }

            uint32_t timestamp = *((const uint32_t*)payload);
            SystemTime_SetTimestamp((uint64_t)timestamp);
            break;
        }
    }
}