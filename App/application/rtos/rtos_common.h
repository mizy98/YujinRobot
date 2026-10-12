#ifndef RTOS_COMMON_H
#define RTOS_COMMON_H

#include "../../domain/interface/compiler_attribute.h"
#include "../../domain/interface/rtos.h"
#include "../../domain/model/app_model.h"

// Forward declarations to break circular dependency
typedef struct Rtos Rtos;

#define TASK_IDLE_STACK_SIZE (1024u * 3u)
#define TASK_RTC_STACK_SIZE (1024u * 3u)
#define TASK_STORAGE_STACK_SIZE (1024u * 16u)
#define TASK_CURRENT_STACK_SIZE (1024u * 4u)
#define TASK_VOLTAGE_STACK_SIZE (1024u * 4u)
#define TASK_TEMPERATURE_STACK_SIZE (1024u * 4u)
#define TASK_PROTECTION_STACK_SIZE (1024u * 4u)
#define TASK_EXTERNAL_COMM_STACK_SIZE (1024u * 8u)
#define TASK_UI_COMM_STACK_SIZE (1024u * 8u)

typedef enum {
    TASK_IDLE_PRIORITY = 8,
    TASK_RTC_PRIORITY,
    TASK_STORAGE_PRIORITY,
    TASK_CURRENT_PRIORITY,
    TASK_VOLTAGE_PRIORITY,
    TASK_TEMPERATURE_PRIORITY,
    TASK_PROTECTION_PRIORITY,
    TASK_EXTERNAL_COMM_PRIORITY,
    TASK_UI_COMM_PRIORITY,
} TaskPriority;

typedef enum {
    TASK_STATE_RESET,
    TASK_STATE_RUN,
    TASK_STATE_ERROR,
} TaskState;

typedef enum {
    EVENT_STORAGE_READY_NONE,
    EVENT_STORAGE_READY_CONFIG = (1 << 0),
} EventStorageReadyFlag;

typedef enum {
    EVENT_SYSTEM_READY_NONE,
    EVENT_SYSTEM_READY_RTC = (1 << 0),
    EVENT_SYSTEM_READY_STORAGE = (1 << 1),
    EVENT_SYSTEM_READY_VOLTAGE = (1 << 2),
    EVENT_SYSTEM_READY_TEMPERATURE = (1 << 3),
    EVENT_SYSTEM_READY_CURRENT = (1 << 4),
} EventSystemReadyFlag;

#define EVENT_SYSTEM_READY_ALL                                                          \
    (EVENT_SYSTEM_READY_RTC | EVENT_SYSTEM_READY_STORAGE | EVENT_SYSTEM_READY_VOLTAGE | \
     EVENT_SYSTEM_READY_TEMPERATURE | EVENT_SYSTEM_READY_CURRENT)

typedef enum {
    EVENT_STORAGE_NONE,
    EVENT_STORAGE_UPDATE_CONFIG = (1 << 0),
    EVENT_STORAGE_RECORD_DATA = (1 << 1),
    EVENT_STORAGE_RECORD_ERROR = (1 << 2),
    EVENT_STORAGE_LOAD_DATA_LOG_INFO = (1 << 3),
    EVENT_STORAGE_LOAD_DATA_LOG_ENTRY = (1 << 4),
    EVENT_STORAGE_LOAD_DATA_LOG = (1 << 5),
    EVENT_STORAGE_RESET_DATA_LOG = (1 << 6),
    EVENT_STORAGE_LOAD_EVENT_LOG_INFO = (1 << 7),
    EVENT_STORAGE_LOAD_EVENT_LOG_ENTRY = (1 << 8),
    EVENT_STORAGE_LOAD_EVENT_LOG = (1 << 9),
    EVENT_STORAGE_RESET_EVENT_LOG = (1 << 10),
} EventStorageFlag;

typedef enum {
    EVENT_UI_COMM_STORAGE_NONE,
    EVENT_UI_COMM_STORAGE_DATA_LOG_ENTRY_READY = (1 << 0),
    EVENT_UI_COMM_STORAGE_DATA_LOG_READY = (1 << 1),
} EventUiCommStorageFlag;

typedef enum {
    EVENT_CONTACTOR_NONE,
    EVENT_CONTACTOR_MAIN_ON = (1 << 0),
    EVENT_CONTACTOR_MAIN_OFF = (1 << 1),
    EVENT_CONTACTOR_CHARGE_ON = (1 << 2),
    EVENT_CONTACTOR_CHARGE_OFF = (1 << 3),
    EVENT_CONTACTOR_DISCHARGE_ON = (1 << 4),
    EVENT_CONTACTOR_DISCHARGE_OFF = (1 << 5),
    EVENT_CONTACTOR_ALL_ON = (1 << 6),
    EVENT_CONTACTOR_ALL_OFF = (1 << 7),
} EventContactorFlag;

typedef enum {
    EVENT_PROTECTION_NONE,
    EVENT_PROTECTION_UPDATE = (1 << 0),
} EventProtectionFlag;

typedef enum {
    EVENT_TEMPERATURE_NONE,
    EVENT_TEMPERATURE_UPDATE_CONFIG = (1 << 0),
} EventTemperatureFlag;

typedef struct {
    TaskState state;
    AppErrorInfo error_info;
    RtosTaskState rtos_state;

    void* handle;
    float stack_usage;
    uint8_t* stack;
    uint32_t stack_size;
} SuperTask;

typedef struct {
    SuperTask super;
} TaskVoltage, TaskCurrent, TaskTemperature, TaskIdle, TaskExternalComm, TaskUiComm, TaskRtc;

typedef struct {
    SuperTask super;
    float data_log_usage;

    struct {
        uint16_t entry_count;
        uint16_t block;
        uint16_t page;
    } data_log_request;

    struct {
        uint16_t entry_count;
        uint16_t block;
        uint16_t page;
    } event_log_request;
} TaskStorage;

typedef struct {
    SuperTask super;

    int32_t protection_snapshot_value;
    int32_t protection_snapshot_threshold;
} TaskProtection;

typedef struct {
    TaskIdle* idle;
    TaskRtc* rtc;
    TaskStorage* storage;
    TaskCurrent* current;
    TaskVoltage* voltage;
    TaskTemperature* temperature;
    TaskProtection* protection;
    TaskExternalComm* external_comm;
    TaskUiComm* ui_comm;
} RtosTask;

typedef struct {
    void* system_ready;
    void* storage;
    void* contactor;
    void* protection;
    void* temperature;
} RtosEvent;

typedef struct {
    void* log_info;
    void* log_data;
    void* data_log_entry;
    void* event_log_entry;
} RtosQueue;

// Rtos structure definition moved to end to resolve circular dependency
struct Rtos {
    App* app;

    RtosTask task;
    RtosEvent event;
    RtosQueue queue;
};

#endif
