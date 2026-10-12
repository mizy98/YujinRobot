#ifndef BMS_INFO_H
#define BMS_INFO_H

#include <stdint.h>
#include <string.h>

#include "ll/app_config.h"

#pragma pack(push, 1)
typedef struct {
    uint16_t info_size;
    uint16_t config_size;
    uint16_t occupied_config_size;
    uint16_t data_size;
    uint16_t brief_data_size;

    char vendor_name[APP_VENDOR_NAME_MAX_LENGTH];
    char project_name[APP_PROJECT_NAME_MAX_LENGTH];

    uint8_t project_version_major;
    uint8_t project_version_minor;
    uint8_t project_version_patch;

    uint32_t serial_number[3];
    uint8_t device_id;

    uint8_t max_bank_voltage_count;

    uint8_t max_pack_temperature_count;
    uint8_t max_onboard_temperature_count;
    uint8_t max_balancer_temperature_count;
    uint8_t max_contactor_temperature_count;

    uint8_t max_heater_count;
    uint8_t max_cooler_count;
} BmsInfo;

#pragma pack(pop)

static inline BmsInfo BmsInfo_CreateDefault(void) {
    BmsInfo info;

    info.info_size = sizeof(BmsInfo);
    info.config_size = sizeof(BmsConfig);
    info.occupied_config_size = BMS_CONFIG_OCCUPIED_SIZE;
    info.data_size = sizeof(BmsData);
    info.brief_data_size = sizeof(BmsBriefData);

    memcpy(info.vendor_name, APP_VENDOR_NAME, strlen(APP_VENDOR_NAME) + 1);
    memcpy(info.project_name, APP_PROJECT_NAME, strlen(APP_PROJECT_NAME) + 1);

    info.project_version_major = APP_PROJECT_VERSION_MAJOR;
    info.project_version_minor = APP_PROJECT_VERSION_MINOR;
    info.project_version_patch = APP_PROJECT_VERSION_PATCH;

    info.serial_number[0] = 0;
    info.serial_number[1] = 0;
    info.serial_number[2] = 0;

    info.device_id = APP_DEVICE_ID;

    info.max_bank_voltage_count = VOLTAGE_MAX_COUNT;
    info.max_pack_temperature_count = TEMPERATURE_MAX_PACK_COUNT;
    info.max_onboard_temperature_count = TEMPERATURE_MAX_BOARD_COUNT;
    info.max_balancer_temperature_count = TEMPERATURE_MAX_BALANCER_COUNT;
    info.max_contactor_temperature_count = TEMPERATURE_MAX_CONTACTOR_COUNT;

    info.max_heater_count = HEATER_MAX_COUNT;
    info.max_cooler_count = FAN_MAX_COUNT;

    return info;
}

#endif