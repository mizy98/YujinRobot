#include "task_storage.h"

#include "../../domain/interface/system.h"
#include "../../domain/interface/system_time.h"
#include "../../domain/service/black_box.h"
#include "../../domain/service/buzzer.h"
#include "../../domain/service/configurator.h"
#include "../../domain/service/nvs.h"

#define self (task_storage)
#define super (task_storage.super)

#define QUEUE_TIMEOUT (1000u)

static TaskStorage task_storage;
static uint8_t _stack[TASK_STORAGE_STACK_SIZE];
static Rtos* _rtos;

static void _onRecordStored(const BmsBriefData* data, uint16_t count) {}

static void _onRecordLoaded(const BmsBriefData* data, uint16_t count) {
    for (uint16_t i = 0; i < count; i++) {
        (void)Rtos_SendQueue(_rtos->queue.log_data, &data[i], QUEUE_TIMEOUT);
    }
}

static void _onRecordLoadFinished(void) {}

static void _onEventRecorded(BmsEventLogType event_type) {
    if ((event_type == BMS_EVENT_LOG_TYPE_BOOT_POWER_ON) ||
        (event_type == BMS_EVENT_LOG_TYPE_BOOT_PIN_RESET) ||
        (event_type == BMS_EVENT_LOG_TYPE_BOOT_SOFTWARE_RESET) ||
        (event_type == BMS_EVENT_LOG_TYPE_BOOT_INDEPENDENT_WATCHDOG_RESET) ||
        (event_type == BMS_EVENT_LOG_TYPE_BOOT_WINDOW_WATCHDOG_RESET) ||
        (event_type == BMS_EVENT_LOG_TYPE_BOOT_LOW_POWER_RESET) ||
        (event_type == BMS_EVENT_LOG_TYPE_BOOT_BROWN_OUT_RESET) ||
        (event_type == BMS_EVENT_LOG_TYPE_BOOT_UNKNOWN_RESET)) {
        System_ClearResetReason();
    }
}

static void _runner(Rtos* rtos) {
    switch (super.state) {
        case TASK_STATE_RESET: {
            uint32_t flag = EVENT_SYSTEM_READY_RTC;
            flag = Rtos_WaitEventNoClear(rtos->event.system_ready, EVENT_SYSTEM_READY_RTC, false,
                                         UINT32_MAX);
            if ((flag & EVENT_SYSTEM_READY_RTC) == 0 || (flag & RTOS_FLAG_ERROR) != 0) {
                break;
            }

            if (Configurator_Initialize() != APP_RESULT_OK) {
                Configurator_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }
            Configurator_Get(&rtos->app->config);

            if (BlackBox_Initialize(
                    &(BlackBoxCallback){
                        .onRecordStored = _onRecordStored,
                        .onRecordLoaded = _onRecordLoaded,
                        .onRecordLoadFinished = _onRecordLoadFinished,
                        .onEventRecorded = _onEventRecorded,
                    },
                    rtos->app->data.timestamp,
                    rtos->app->config.misc.time_zone_offset_hours) != APP_RESULT_OK) {
                BlackBox_GetLastErrorInfo(&super.error_info);
                super.state = TASK_STATE_ERROR;
                break;
            }

            Rtos_SetEvent(rtos->event.system_ready, EVENT_SYSTEM_READY_STORAGE);

            super.state = TASK_STATE_RUN;
            break;
        }

        case TASK_STATE_RUN: {
            uint32_t flag = EVENT_STORAGE_UPDATE_CONFIG | EVENT_STORAGE_RECORD_DATA |
                            EVENT_STORAGE_RECORD_ERROR | EVENT_STORAGE_LOAD_DATA_LOG_ENTRY |
                            EVENT_STORAGE_LOAD_DATA_LOG_INFO | EVENT_STORAGE_LOAD_DATA_LOG |
                            EVENT_STORAGE_RESET_DATA_LOG | EVENT_STORAGE_LOAD_EVENT_LOG_INFO |
                            EVENT_STORAGE_LOAD_EVENT_LOG_ENTRY | EVENT_STORAGE_LOAD_EVENT_LOG |
                            EVENT_STORAGE_RESET_EVENT_LOG;
            flag = Rtos_WaitEvent(rtos->event.storage, flag, true, UINT32_MAX);
            if ((flag & RTOS_FLAG_ERROR) != 0) {
                break;
            }

            if (flag & EVENT_STORAGE_UPDATE_CONFIG) {
                if (Configurator_Save(&rtos->app->config) != APP_RESULT_OK) {
                    Configurator_GetLastErrorInfo(&super.error_info);
                    super.state = TASK_STATE_ERROR;
                    break;
                }

                (void)Buzzer_Beep(BUZZER_TYPE_STORAGE_UPDATED);
            }

            if (flag & EVENT_STORAGE_RECORD_DATA) {
                BmsData data = rtos->app->data;
                int32_t protection_snapshot_value =
                    rtos->task.protection->protection_snapshot_value;
                int32_t protection_snapshot_threshold =
                    rtos->task.protection->protection_snapshot_threshold;

                if (BlackBox_RecordData(&data, &self.data_log_usage, protection_snapshot_value,
                                        protection_snapshot_threshold) != APP_RESULT_OK) {
                    BlackBox_GetLastErrorInfo(&super.error_info);
                    super.state = TASK_STATE_ERROR;
                    break;
                }

                BmsDataLogEntry current_data_log_entry = {0};
                if (BlackBox_GetCurrentDataLogEntry(&current_data_log_entry) == APP_RESULT_OK) {
                    rtos->app->data.data_log_entry = current_data_log_entry;
                }
            }

            if (flag & EVENT_STORAGE_RECORD_ERROR) {
            }

            if (flag & EVENT_STORAGE_LOAD_DATA_LOG_ENTRY) {
                uint8_t request_count = self.data_log_request.entry_count;

                BmsDataLogEntry dummy;
                while (Rtos_ReceiveQueue(rtos->queue.data_log_entry, &dummy, 0u) == APP_RESULT_OK) {
                }

                if (request_count != 0u) {
                    BmsDataLogEntry entries[DATA_LOG_MAX_COUNT];
                    uint8_t count = 0;

                    if (BlackBox_GetDataLogEntryHistory(entries, request_count, &count) !=
                        APP_RESULT_OK) {
                        BlackBox_GetLastErrorInfo(&super.error_info);
                        super.state = TASK_STATE_ERROR;
                        break;
                    }

                    for (uint8_t i = 0; i < count; i++) {
                        if (Rtos_SendQueue(rtos->queue.data_log_entry, &entries[i],
                                           QUEUE_TIMEOUT) != APP_RESULT_OK) {
                            APP_ERROR_SET_INFO(super.error_info,
                                               APP_ERROR_TASK_STORAGE_SEND_DATA_LOG_ENTRY_QUEUE);
                            super.state = TASK_STATE_ERROR;
                            break;
                        }
                    }
                }
            }

            if (flag & EVENT_STORAGE_LOAD_DATA_LOG_INFO) {
                BmsDataLogInfo info;

                if (BlackBox_GetDataLogInfo(&info) != APP_RESULT_OK) {
                    BlackBox_GetLastErrorInfo(&super.error_info);
                    super.state = TASK_STATE_ERROR;
                    break;
                }

                if (Rtos_SendQueue(rtos->queue.log_info, &info, QUEUE_TIMEOUT) != APP_RESULT_OK) {
                    APP_ERROR_SET_INFO(super.error_info,
                                       APP_ERROR_TASK_STORAGE_SEND_DATA_LOG_INFO_QUEUE);
                    super.state = TASK_STATE_ERROR;
                    break;
                }
            }

            if (flag & EVENT_STORAGE_LOAD_DATA_LOG) {
                uint16_t block = self.data_log_request.block;
                uint16_t page = self.data_log_request.page;
                uint8_t data[DATA_LOG_MAX_PAGE_SIZE] = {0};

                if (BlackBox_ReadDataLog(data, sizeof(data), block, page) != APP_RESULT_OK) {
                    BlackBox_GetLastErrorInfo(&super.error_info);
                    break;
                }

                Rtos_SendQueue(rtos->queue.log_data, data, QUEUE_TIMEOUT);
            }

            if (flag & EVENT_STORAGE_RESET_DATA_LOG) {
                if (BlackBox_EraseAllDataLogs() != APP_RESULT_OK) {
                    BlackBox_GetLastErrorInfo(&super.error_info);
                    super.state = TASK_STATE_ERROR;
                    break;
                }
            }

            if (flag & EVENT_STORAGE_LOAD_EVENT_LOG_INFO) {
                BmsEventLogInfo info;
                if (BlackBox_GetEventLogInfo(&info) != APP_RESULT_OK) {
                    BlackBox_GetLastErrorInfo(&super.error_info);
                    super.state = TASK_STATE_ERROR;
                    break;
                }

                if (Rtos_SendQueue(rtos->queue.log_info, &info, QUEUE_TIMEOUT) != APP_RESULT_OK) {
                    APP_ERROR_SET_INFO(super.error_info,
                                       APP_ERROR_TASK_STORAGE_SEND_EVENT_LOG_INFO_QUEUE);
                    super.state = TASK_STATE_ERROR;
                    break;
                }
            }

            if (flag & EVENT_STORAGE_LOAD_EVENT_LOG_ENTRY) {
                uint8_t request_count = self.event_log_request.entry_count;

                BmsEventLogEntry dummy;
                while (Rtos_ReceiveQueue(rtos->queue.event_log_entry, &dummy, 0u) ==
                       APP_RESULT_OK) {
                }

                if (request_count != 0u) {
                    BmsEventLogEntry entries[EVENT_LOG_MAX_COUNT];
                    uint8_t count = 0;

                    if (BlackBox_GetEventLogEntryHistory(entries, request_count, &count) !=
                        APP_RESULT_OK) {
                        BlackBox_GetLastErrorInfo(&super.error_info);
                        super.state = TASK_STATE_ERROR;
                        break;
                    }

                    for (uint8_t i = 0; i < count; i++) {
                        if (Rtos_SendQueue(rtos->queue.event_log_entry, &entries[i],
                                           QUEUE_TIMEOUT) != APP_RESULT_OK) {
                            APP_ERROR_SET_INFO(super.error_info,
                                               APP_ERROR_TASK_STORAGE_SEND_EVENT_LOG_ENTRY_QUEUE);
                            super.state = TASK_STATE_ERROR;
                            break;
                        }
                    }
                }
            }

            if (flag & EVENT_STORAGE_LOAD_EVENT_LOG) {
                uint16_t block = self.event_log_request.block;
                uint16_t page = self.event_log_request.page;
                uint8_t data[DATA_LOG_MAX_PAGE_SIZE] = {0};

                if (BlackBox_ReadEventLog(data, sizeof(data), block, page) != APP_RESULT_OK) {
                    BlackBox_GetLastErrorInfo(&super.error_info);
                    break;
                }

                Rtos_SendQueue(rtos->queue.log_data, data, QUEUE_TIMEOUT);
            }

            if (flag & EVENT_STORAGE_RESET_EVENT_LOG) {
                if (BlackBox_EraseAllEventLogs() != APP_RESULT_OK) {
                    BlackBox_GetLastErrorInfo(&super.error_info);
                    super.state = TASK_STATE_ERROR;
                    break;
                }
            }

            break;
        }

        case TASK_STATE_ERROR: {
            Rtos_SleepMs(1000u);

            (void)Configurator_Deinitialize();
            (void)BlackBox_Deinitialize();

            rtos->app->data.last_error_code = super.error_info.code;

            super.state = TASK_STATE_RESET;
            break;
        }
    }
}

static void _wrapper(void* argument) {
    while (true) {
        _runner((Rtos*)argument);
    }
}

AppResult TaskStorage_Initialize(Rtos* rtos) {
    void* task = Rtos_CreateStaticTask(_wrapper, rtos, "storage", _stack, sizeof(_stack),
                                       TASK_STORAGE_PRIORITY);
    if (task == NULL) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_STORAGE_CREATE);
        return APP_RESULT_FAIL;
    }

    super.handle = task;
    super.state = TASK_STATE_RESET;
    super.error_info.code = APP_ERROR_NONE;
    super.stack_usage = 0.0f;
    super.stack = _stack;
    super.stack_size = sizeof(_stack);

    _rtos = rtos;
    rtos->task.storage = &self;

    return APP_RESULT_OK;
}

AppResult TaskStorage_Deinitialize(void) {
    if (Rtos_DeleteTask(super.handle)) {
        APP_ERROR_SET_INFO(super.error_info, APP_ERROR_TASK_STORAGE_DELETE);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void TaskStorage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = super.error_info;
    }
}
