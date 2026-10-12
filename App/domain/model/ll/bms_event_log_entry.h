#ifndef BMS_EVENT_LOG_ENTRY_H
#define BMS_EVENT_LOG_ENTRY_H

#include <stdint.h>

enum BmsEventLogType {
    BMS_EVENT_LOG_TYPE_NONE,
    BMS_EVENT_LOG_TYPE_BOOT_POWER_ON,
    BMS_EVENT_LOG_TYPE_BOOT_PIN_RESET,
    BMS_EVENT_LOG_TYPE_BOOT_SOFTWARE_RESET,
    BMS_EVENT_LOG_TYPE_BOOT_INDEPENDENT_WATCHDOG_RESET,
    BMS_EVENT_LOG_TYPE_BOOT_WINDOW_WATCHDOG_RESET,
    BMS_EVENT_LOG_TYPE_BOOT_LOW_POWER_RESET,
    BMS_EVENT_LOG_TYPE_BOOT_BROWN_OUT_RESET,
    BMS_EVENT_LOG_TYPE_BOOT_UNKNOWN_RESET,
    BMS_EVENT_LOG_TYPE_PROTECTION_OCCURRED,
    BMS_EVENT_LOG_TYPE_PROTECTION_RELEASED,
    BMS_EVENT_LOG_TYPE_SLEEP_ENTER,
    BMS_EVENT_LOG_TYPE_SLEEP_EXIT,
};
typedef uint8_t BmsEventLogType;

#pragma pack(push, 1)
typedef struct {
    uint64_t datetime_id;  // (YYYYMMDDHHMMSS)
    BmsEventLogType type;
    uint32_t sequence;
    uint32_t record_count;
    struct {
        uint16_t block;
        uint16_t page;
    } start_address;
    struct {
        uint16_t block;
        uint16_t page;
    } end_address;
    uint32_t reset_flag;  // raw reset flag value for boot events, 0 for non-boot events

    struct {
        uint32_t flag;  // raw protection flag value for protection events, 0 for
                        // non-protection events
        int32_t snapshot_value;
        uint32_t snapshot_threshold;
    } protection;

} BmsEventLogEntry;
#pragma pack(pop)

#endif
