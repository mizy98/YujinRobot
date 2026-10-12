#ifndef BMS_DATA_H
#define BMS_DATA_H

#include "ll/app_error.h"
#include "ll/bms_current_data.h"
#include "ll/bms_data_log_entry.h"
#include "ll/bms_expansion_data.h"
#include "ll/bms_protection_state.h"
#include "ll/bms_soc_data.h"
#include "ll/bms_temperature_data.h"
#include "ll/bms_type.h"
#include "ll/bms_voltage_data.h"

#pragma pack(push, 1)

// TODO: rename structure
typedef struct {
    uint32_t timestamp;                                 // byte offset: 0
    BmsSocData soc;                                     // byte offset: 4
    BmsPackState pack_state;                            // byte offset: 18
    BmsVoltageData voltage;                             // byte offset: 19
    BmsCurrentData current;                             // byte offset: 63
    BmsPackTemperatureData pack_temperature;            // byte offset: 75
    BmsBoardTemperatureData board_temperature;          // byte offset: 93
    BmsBalancerTemperatureData balancer_temperature;    // byte offset: 107
    BmsContactorTemperatureData contactor_temperature;  // byte offset: 119
    BmsProtectionState protection_state;                // byte offset: 131
    BmsProtectionState warning_state;                   // byte offset: 135
    BmsProtectionState pf_state;                        // byte offset: 139
    BmsContactorState main_contactor_state;             // byte offset: 143
    BmsContactorState charge_contactor_state;           // byte offset: 144
    BmsContactorState discharge_contactor_state;        // byte offset: 145
    BmsDeviceState heater_state;                        // byte offset: 146
    BmsDeviceState cooler_state;                        // byte offset: 147
    uint16_t pressure;                                  // byte offset: 148 (0.1 PSI)
    uint32_t last_error_code;                           // byte offset: 150
    BmsDataLogEntry data_log_entry;                     // byte offset: 154
    BmsExpansionData expansion;                         // byte offset: 174
} BmsData;

typedef struct {
    uint32_t timestamp;
    uint16_t real_soc;                                      // 0.1%
    uint16_t display_soc;                                   // 0.1%
    uint16_t bank_voltages[VOLTAGE_MAX_COUNT];              // mV
    int16_t current;                                        // 0.1A
    int16_t pack_temperatures[TEMPERATURE_MAX_PACK_COUNT];  // 0.1℃
    BmsProtectionState protection_state;
    BmsProtectionState warning_state;
    uint16_t current_offset_voltage;  // mV
    uint8_t reserved[1];

    BmsPackState pack_state : 6;
    BmsContactorState main_contactor_state : 2;

    BmsContactorState charge_contactor_state : 2;
    BmsContactorState discharge_contactor_state : 2;
    BmsContactorState precharge_contactor_state : 2;
    BmsDeviceState heater_state : 2;

    BmsDeviceState cooler_state : 2;
} BmsBriefData;

#pragma pack(pop)

#endif
