#include "rtos_startup.h"

#include "../rtos/rtos_common.h"
#include "../rtos/task_current.h"
#include "../rtos/task_external_comm.h"
#include "../rtos/task_idle.h"
#include "../rtos/task_protection.h"
#include "../rtos/task_rtc.h"
#include "../rtos/task_storage.h"
#include "../rtos/task_temperature.h"
#include "../rtos/task_ui_comm.h"
#include "../rtos/task_voltage.h"

#define self (startup_private)

static struct {
    AppErrorInfo error_info;
} startup_private;

static Rtos rtos;

// buffers for data & event log
static uint8_t log_info_buffer[sizeof(BmsDataLogInfo) + 80];
static uint8_t log_record_buffer[DATA_LOG_MAX_PAGE_SIZE + 80];

// buffer for data log
static uint8_t data_log_entry_buffer[DATA_LOG_MAX_COUNT * sizeof(BmsDataLogEntry) + 80];

// buffer for event log
static uint8_t event_log_entry_buffer[EVENT_LOG_MAX_COUNT * sizeof(BmsEventLogEntry) + 80];

AppResult RtosStartup_Initialize(App* app) {
    rtos.app = app;

    if (Rtos_Initialize() != APP_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_STARTUP_RTOS_INIT);
        return APP_RESULT_FAIL;
    }

    if (TaskIdle_Initialize(&rtos) != APP_RESULT_OK) {
        TaskIdle_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (TaskRtc_Initialize(&rtos) != APP_RESULT_OK) {
        TaskRtc_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (TaskStorage_Initialize(&rtos) != APP_RESULT_OK) {
        TaskStorage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (TaskCurrent_Initialize(&rtos) != APP_RESULT_OK) {
        TaskCurrent_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (TaskVoltage_Initialize(&rtos) != APP_RESULT_OK) {
        TaskVoltage_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (TaskTemperature_Initialize(&rtos) != APP_RESULT_OK) {
        TaskTemperature_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (TaskProtection_Initialize(&rtos) != APP_RESULT_OK) {
        TaskProtection_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (TaskExternalComm_Initialize(&rtos) != APP_RESULT_OK) {
        TaskExternalComm_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    if (TaskUiComm_Initialize(&rtos) != APP_RESULT_OK) {
        TaskUiComm_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    void* event = NULL;
    event = Rtos_CreateEvent();
    if (event == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_STARTUP_RTOS_CREATE_EVENT);
        return APP_RESULT_FAIL;
    }
    rtos.event.system_ready = event;

    event = Rtos_CreateEvent();
    if (event == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_STARTUP_RTOS_CREATE_EVENT);
        return APP_RESULT_FAIL;
    }
    rtos.event.storage = event;

    event = Rtos_CreateEvent();
    if (event == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_STARTUP_RTOS_CREATE_EVENT);
        return APP_RESULT_FAIL;
    }
    rtos.event.contactor = event;

    event = Rtos_CreateEvent();
    if (event == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_STARTUP_RTOS_CREATE_EVENT);
        return APP_RESULT_FAIL;
    }
    rtos.event.protection = event;

    void* queue = NULL;
    queue =
        Rtos_CreateStaticQueue(sizeof(BmsDataLogInfo), 1, log_info_buffer, sizeof(log_info_buffer));
    if (queue == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_STARTUP_RTOS_CREATE_QUEUE);
        return APP_RESULT_FAIL;
    }
    rtos.queue.log_info = queue;

    queue = Rtos_CreateStaticQueue(DATA_LOG_MAX_PAGE_SIZE, 1, log_record_buffer,
                                   sizeof(log_record_buffer));
    if (queue == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_STARTUP_RTOS_CREATE_QUEUE);
        return APP_RESULT_FAIL;
    }
    rtos.queue.log_data = queue;

    queue = Rtos_CreateStaticQueue(sizeof(BmsDataLogEntry), DATA_LOG_MAX_COUNT,
                                   data_log_entry_buffer, sizeof(data_log_entry_buffer));
    if (queue == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_STARTUP_RTOS_CREATE_QUEUE);
        return APP_RESULT_FAIL;
    }
    rtos.queue.data_log_entry = queue;

    queue = Rtos_CreateStaticQueue(sizeof(BmsEventLogEntry), EVENT_LOG_MAX_COUNT,
                                   event_log_entry_buffer, sizeof(event_log_entry_buffer));
    if (queue == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_STARTUP_RTOS_CREATE_QUEUE);
        return APP_RESULT_FAIL;
    }
    rtos.queue.event_log_entry = queue;

    if (Rtos_StartScheduler() != APP_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_STARTUP_RTOS_START_SCHEDULER);
        return APP_RESULT_FAIL;
    }

    // not reached if scheduler starts successfully
    APP_ERROR_SET_INFO(self.error_info, APP_ERROR_STARTUP_RTOS_START_SCHEDULER);

    return APP_RESULT_FAIL;
}
