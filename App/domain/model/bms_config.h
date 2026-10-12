#ifndef BMS_CONFIG_H
#define BMS_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ll/app_config.h"
#include "ll/bms_cell_model.h"
#include "ll/bms_protection_config.h"
#include "ll/bms_type.h"

#define BMS_CONFIG_MAX_SIZE (1024u)
#define BMS_CONFIG_OCCUPIED_SIZE                                                        \
    (sizeof(BmsConfigMeta) + APP_PRODUCT_NAME_MAX_LENGTH + sizeof(BmsPackConfig) +      \
     sizeof(BmsCurrentConfig) + sizeof(BmsStateConfig) + sizeof(BmsSocConfig) +         \
     sizeof(BmsThermalConfig) + sizeof(BmsContactorConfig) +                            \
     sizeof(BmsProtectionConfig) * BMS_PROTECTION_CODE_MAX + sizeof(BmsChargerConfig) + \
     sizeof(BmsSleepConfig) + sizeof(BmsBalancingConfig) + sizeof(BmsFanConfig) +       \
     sizeof(BmsHeaterConfig) + sizeof(BmsMiscConfig))

#pragma pack(push, 1)
typedef struct {
    uint8_t version;
    uint32_t timestamp;
    uint16_t sequence;
    uint16_t size;
} BmsConfigMeta;

typedef struct {
    BmsCellModel cell_model;
    uint16_t serial_count;
    uint16_t parallel_count;
} BmsPackConfig;

typedef struct {
    uint8_t direction;          // 0: backward, 1: forward
    uint32_t calibration_tick;  // ms
    uint16_t ema_alpha;         // 0.001 (0 ~ 1000), fixed-point with 3 decimal places
} BmsCurrentConfig;

typedef struct {
    BmsChargeParseMode charge_parse_mode;
    int16_t charge_threshold_current;     // 0.1A
    int16_t discharge_threshold_current;  // 0.1A

    // used when charge_parse_mode is BMS_CHARGE_PARSE_MODE_TIME
    // or BMS_CHARGE_PARSE_MODE_SIGNAL
    uint32_t charge_threshold_tick;  // ms

    // used when charge_parse_mode is BMS_CHARGE_PARSE_MODE_CC
    uint32_t cc_parse_sampling_tick;            // ms
    uint32_t cc_parse_threshold_tick;           // ms
    int16_t cc_parse_stable_threshold_current;  // 0.1A

    uint8_t reserved[16];
} BmsStateConfig;

typedef struct {
    uint32_t ocv_calibration_tick;       // ms
    uint32_t capacity_calibration_tick;  // ms
    uint16_t track_unit;                 // 0.1%
    uint32_t track_tick;                 // ms
} BmsSocConfig;

typedef struct {
    uint8_t pack_temperature_count;
    uint8_t board_temperature_count;
    uint8_t balancer_temperature_count;
    uint8_t contactor_temperature_count;
} BmsThermalConfig;

typedef struct {
    uint32_t main_contactor_precharge_tick;       // ms, 0 means no precharge
    uint32_t charge_contactor_precharge_tick;     // ms, 0 means no precharge
    uint32_t discharge_contactor_precharge_tick;  // ms, 0 means no precharge

    uint32_t main_contactor_check_threshold_tick;       // ms, 0 means no check
    uint32_t charge_contactor_check_threshold_tick;     // ms, 0 means no check
    uint32_t discharge_contactor_check_threshold_tick;  // ms, 0 means no check

    uint8_t mode;
    uint8_t reserved[3];
} BmsContactorConfig;

typedef struct {
    BmsChargerMode mode;
    uint16_t cc_current;              // 0.1A
    uint16_t cv_voltage;              // 0.1V
    uint16_t cv_cutoff_current;       // 0.1A
    uint16_t recharge_start_voltage;  // 0.1V
} BmsChargerConfig;

typedef struct {
    BmsSleepMode mode;
    uint32_t threshold_tick;     // ms
    uint16_t threshold_soc;      // 0.1%, only used when in BMS_SLEEP_MODE_SOC
    uint16_t threshold_voltage;  // mV, only used when in BMS_SLEEP_MODE_VOLTAGE
} BmsSleepConfig;

typedef struct {
    bool is_enabled;
    uint16_t threshold_dv;            // mV
    uint16_t threshold_bank_voltage;  // mV
    uint32_t threshold_tick;          // ms
    uint16_t release_threshold_dv;    // mV
    uint32_t release_threshold_tick;  // ms
} BmsBalancingConfig;

typedef struct {
    bool is_enabled;
    uint16_t threshold_soc;
    int16_t threshold_value;          // 0.1℃
    uint32_t threshold_tick;          // ms
    int16_t release_threshold_value;  // 0.1℃
    uint32_t release_threshold_tick;  // ms
} BmsFanConfig, BmsHeaterConfig;

typedef struct {
    struct {
        uint32_t uvp_delay_tick;  // ms
        uint16_t odvp_threshold;  // mV
    } protection_extension;

    struct {
        bool is_enabled;          // ms
        uint32_t threshold_tick;  // ms
    } external_contactor;

    struct {
        BmsBdiType type;  // 0: disabled, 1: mzn58 can
        uint16_t add_value;
    } bdi;

    uint8_t time_zone_offset_hours;  // Time zone offset in hours (0-14)

    bool is_tago_application;
    bool is_tago_lsv_application;
    bool is_global_tech_application;
    bool is_alwayz_application;
    bool is_hgp_application;

} BmsMiscConfig;

typedef struct {
    BmsConfigMeta meta;

    char product_name[APP_PRODUCT_NAME_MAX_LENGTH];

    BmsPackConfig pack;
    BmsCurrentConfig current;
    BmsStateConfig state;
    BmsSocConfig soc;
    BmsThermalConfig thermal;
    BmsContactorConfig contactor;

    union {
        struct {
            BmsProtectionConfig ovp;
            BmsProtectionConfig uvp;
            BmsProtectionConfig cocp;
            BmsProtectionConfig docp;
            BmsProtectionConfig rocp;
            BmsProtectionConfig otp;
            BmsProtectionConfig cotp;
            BmsProtectionConfig dotp;
            BmsProtectionConfig rotp;
            BmsProtectionConfig utp;
            BmsProtectionConfig cutp;
            BmsProtectionConfig dutp;
            BmsProtectionConfig rutp;
            BmsProtectionConfig odvp;
            BmsProtectionConfig codvp;
            BmsProtectionConfig dodvp;
            BmsProtectionConfig rodvp;
            BmsProtectionConfig odtp;
            BmsProtectionConfig opp;
            BmsProtectionConfig upp;
        };
        BmsProtectionConfig entries[BMS_PROTECTION_CODE_MAX];
    } protection;

    BmsChargerConfig charger;
    BmsSleepConfig sleep;
    BmsBalancingConfig balancing;
    BmsFanConfig fan;
    BmsHeaterConfig heater;
    BmsMiscConfig misc;

    uint8_t reserved[BMS_CONFIG_MAX_SIZE - BMS_CONFIG_OCCUPIED_SIZE];
} BmsConfig;

#pragma pack(pop)

// Do not use this function directly, use BmsConfig_CreateDefault() instead
static inline void _BmsConfig_SetCommon(BmsConfig* config) {
    // meta
    config->meta.version = APP_CONFIG_VERSION;
    config->meta.timestamp = 0;
    config->meta.sequence = 0;
    config->meta.size = BMS_CONFIG_OCCUPIED_SIZE;

    // strings
    memset(config->product_name, 0, sizeof(config->product_name));
    snprintf(config->product_name, sizeof(config->product_name), "%s", APP_DEFAULT_PRODUCT_NAME);

    // current
    config->current.direction = CURRENT_DIRECTION;
    config->current.calibration_tick = CURRENT_CALIBRATION_DEFAULT_TICK;
    config->current.ema_alpha = CURRENT_EMA_ALPHA;

    // state
    config->state.charge_parse_mode = STATE_DEFAULT_CHARGE_PARSE_MODE;
    config->state.charge_threshold_current = STATE_DEFAULT_CHARGE_THRESHOLD_CURRENT;
    config->state.discharge_threshold_current = STATE_DEFAULT_DISCHARGE_THRESHOLD_CURRENT;
    config->state.charge_threshold_tick = STATE_DEFAULT_CHARGE_THRESHOLD_TICK;
    config->state.cc_parse_sampling_tick = STATE_CC_SAMPLE_TICK;
    config->state.cc_parse_threshold_tick = STATE_CC_THRESHOLD_TICK;
    config->state.cc_parse_stable_threshold_current = STATE_CC_STABLE_THRESHOLD_CURRENT;

    // soc
    config->soc.track_unit = SOC_DEFAULT_TRACK_UNIT;
    config->soc.track_tick = SOC_DEFAULT_TRACK_TICK;

    // thermal
    config->thermal.pack_temperature_count = TEMPERATURE_DEFAULT_PACK_COUNT;
    config->thermal.board_temperature_count = TEMPERATURE_DEFAULT_BOARD_COUNT;
    config->thermal.balancer_temperature_count = TEMPERATURE_DEFAULT_BALANCER_COUNT;
    config->thermal.contactor_temperature_count = TEMPERATURE_DEFAULT_CONTACTOR_COUNT;

    // contactor
    config->contactor.main_contactor_check_threshold_tick = CONTACTOR_MAIN_DEFAULT_CHECK_TICK;
    config->contactor.charge_contactor_check_threshold_tick = CONTACTOR_CHARGE_DEFAULT_CHECK_TICK;
    config->contactor.discharge_contactor_check_threshold_tick =
        CONTACTOR_DISCHARGE_DEFAULT_CHECK_TICK;
    config->contactor.main_contactor_precharge_tick = CONTACTOR_MAIN_DEFAULT_PRECHARGE_TICK;
    config->contactor.charge_contactor_precharge_tick = CONTACTOR_CHARGE_DEFAULT_PRECHARGE_TICK;
    config->contactor.discharge_contactor_precharge_tick =
        CONTACTOR_DISCHARGE_DEFAULT_PRECHARGE_TICK;
    config->contactor.mode = CONTACTOR_DEFAULT_MODE;

    // protection
    for (uint8_t i = 0; i < BMS_PROTECTION_CODE_MAX; i++) {
        config->protection.entries[i].code = i;
    }

    // charger
    config->charger.mode = CHARGER_DEFAULT_MODE;
    config->charger.cc_current = CHARGER_DEFAULT_CC_CURRENT;
    config->charger.cv_cutoff_current = CHARGER_DEFAULT_CV_CUTOFF_CURRENT;

    // sleep
    config->sleep.mode = BMS_SLEEP_MODE_SOC;
    config->sleep.threshold_tick = STATE_DEFAULT_SLEEP_THRESHOLD_TICK;
    config->sleep.threshold_soc = STATE_DEFAULT_SLEEP_THRESHOLD_SOC;
    config->sleep.threshold_voltage = STATE_DEFAULT_SLEEP_THRESHOLD_VOLTAGE;

    // balancing
    config->balancing.is_enabled = BALANCING_DEFAULT_ENABLED;
    config->balancing.threshold_tick = BALANCING_DEFAULT_THRESHOLD_TICK;
    config->balancing.release_threshold_tick = BALANCING_DEFAULT_RELEASE_THRESHOLD_TICK;

    // fan
    config->fan.is_enabled = FAN_DEFAULT_ENABLED;
    config->fan.threshold_soc = FAN_DEFAULT_THRESHOLD_SOC;
    config->fan.threshold_value = FAN_DEFAULT_THRESHOLD_VALUE;
    config->fan.threshold_tick = FAN_DEFAULT_THRESHOLD_TICK;
    config->fan.release_threshold_value = FAN_DEFAULT_RELEASE_THRESHOLD_VALUE;
    config->fan.release_threshold_tick = FAN_DEFAULT_RELEASE_THRESHOLD_TICK;

    // heater
    config->heater.is_enabled = HEATER_DEFAULT_ENABLED;
    config->heater.threshold_soc = HEATER_DEFAULT_THRESHOLD_SOC;
    config->heater.threshold_value = HEATER_DEFAULT_THRESHOLD_VALUE;
    config->heater.threshold_tick = HEATER_DEFAULT_THRESHOLD_TICK;
    config->heater.release_threshold_value = HEATER_DEFAULT_RELEASE_THRESHOLD_VALUE;
    config->heater.release_threshold_tick = HEATER_DEFAULT_RELEASE_THRESHOLD_TICK;

    // misc
    config->misc.protection_extension.uvp_delay_tick = PROTECTION_DEFAULT_UVP_DELAY_TICK;
    config->misc.external_contactor.is_enabled = CONTACTOR_EXTERNAL_DEFAULT_ENABLED;
    config->misc.external_contactor.threshold_tick = CONTACTOR_EXTERNAL_DEFAULT_THRESHOLD_TICK;
    config->misc.time_zone_offset_hours = DATA_LOG_DEFAULT_TIME_ZONE_OFFSET_HOURS;
    config->misc.bdi.type = BDI_DEFAULT_TYPE;
    config->misc.time_zone_offset_hours = 9;  // UTC+9 KST
    config->misc.is_tago_application = false;
    config->misc.is_tago_lsv_application = false;
    config->misc.is_global_tech_application = false;
    config->misc.is_alwayz_application = false;
    config->misc.is_hgp_application = true;
}

// Do not use this function directly, use BmsConfig_CreateDefault() instead
static inline void _BmsConfig_SetNcm(BmsConfig* config) {
    // pack
#if PACK_NCM_DEFAULT_MODEL == 0
    config->pack.cell_model = BMS_CELL_MODEL_LGES_INR21700_M50LT;
#elif PACK_NCM_DEFAULT_MODEL == 1
    config->pack.cell_model = BMS_CELL_MODEL_SKON_E600;
#else
    config->pack.cell_model = BMS_CELL_MODEL_LGES_INR21700_M50LT;
#endif

    config->pack.parallel_count = PACK_NCM_DEFAULT_PARALLEL_COUNT;
    config->pack.serial_count = PACK_NCM_DEFAULT_SERIAL_COUNT;

    // soc
    config->soc.ocv_calibration_tick = SOC_NCM_DEFAULT_OCV_CALIBRATION_TICK;
    config->soc.capacity_calibration_tick = SOC_NCM_DEFAULT_CAPACITY_CALIBRATION_TICK;

    // charger
    config->charger.cv_voltage = CHARGER_NCM_DEFAULT_CV_VOLTAGE;
    config->charger.recharge_start_voltage = CHARGER_NCM_DEFAULT_RECHARGE_START_VOLTAGE;

    // balancing
    config->balancing.threshold_dv = BALANCING_DEFAULT_NCM_THRESHOLD_DV;
    config->balancing.threshold_bank_voltage = BALANCING_DEFAULT_NCM_THRESHOLD_BANK_VOLTAGE;
    config->balancing.release_threshold_dv = BALANCING_DEFAULT_NCM_RELEASE_THRESHOLD_DV;

    // protection
    // OVP
    config->protection.ovp.is_enabled = PROTECTION_NCM_OVP_DEFAULT_ENABLED;
    config->protection.ovp.threshold_value = PROTECTION_NCM_OVP_DEFAULT_THRESHOLD;
    config->protection.ovp.threshold_tick = PROTECTION_NCM_OVP_DEFAULT_THRESHOLD_TICK;
    config->protection.ovp.is_release_enabled = PROTECTION_NCM_OVP_DEFAULT_RELEASE_ENABLED;
    config->protection.ovp.release_threshold_value = PROTECTION_NCM_OVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.ovp.release_threshold_tick =
        PROTECTION_NCM_OVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.ovp.is_pf_enabled = PROTECTION_NCM_OVP_DEFAULT_PF_ENABLED;

    // UVP
    config->protection.uvp.is_enabled = PROTECTION_NCM_UVP_DEFAULT_ENABLED;
    config->protection.uvp.threshold_value = PROTECTION_NCM_UVP_DEFAULT_THRESHOLD;
    config->protection.uvp.threshold_tick = PROTECTION_NCM_UVP_DEFAULT_THRESHOLD_TICK;
    config->protection.uvp.is_release_enabled = PROTECTION_NCM_UVP_DEFAULT_RELEASE_ENABLED;
    config->protection.uvp.release_threshold_value = PROTECTION_NCM_UVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.uvp.release_threshold_tick =
        PROTECTION_NCM_UVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.uvp.is_pf_enabled = PROTECTION_NCM_UVP_DEFAULT_PF_ENABLED;

    // COCP
    config->protection.cocp.is_enabled = PROTECTION_NCM_COCP_DEFAULT_ENABLED;
    config->protection.cocp.threshold_value = PROTECTION_NCM_COCP_DEFAULT_THRESHOLD;
    config->protection.cocp.threshold_tick = PROTECTION_NCM_COCP_DEFAULT_THRESHOLD_TICK;
    config->protection.cocp.is_release_enabled = PROTECTION_NCM_COCP_DEFAULT_RELEASE_ENABLED;
    config->protection.cocp.release_threshold_value = PROTECTION_NCM_COCP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.cocp.release_threshold_tick =
        PROTECTION_NCM_COCP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.cocp.is_pf_enabled = PROTECTION_NCM_COCP_DEFAULT_PF_ENABLED;

    // DOCP
    config->protection.docp.is_enabled = PROTECTION_NCM_DOCP_DEFAULT_ENABLED;
    config->protection.docp.threshold_value = PROTECTION_NCM_DOCP_DEFAULT_THRESHOLD;
    config->protection.docp.threshold_tick = PROTECTION_NCM_DOCP_DEFAULT_THRESHOLD_TICK;
    config->protection.docp.is_release_enabled = PROTECTION_NCM_DOCP_DEFAULT_RELEASE_ENABLED;
    config->protection.docp.release_threshold_value = PROTECTION_NCM_DOCP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.docp.release_threshold_tick =
        PROTECTION_NCM_DOCP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.docp.is_pf_enabled = PROTECTION_NCM_DOCP_DEFAULT_PF_ENABLED;

    // ROCP
    config->protection.rocp.is_enabled = PROTECTION_NCM_ROCP_DEFAULT_ENABLED;
    config->protection.rocp.threshold_value = PROTECTION_NCM_ROCP_DEFAULT_THRESHOLD;
    config->protection.rocp.threshold_tick = PROTECTION_NCM_ROCP_DEFAULT_THRESHOLD_TICK;
    config->protection.rocp.is_release_enabled = PROTECTION_NCM_ROCP_DEFAULT_RELEASE_ENABLED;
    config->protection.rocp.release_threshold_value = PROTECTION_NCM_ROCP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.rocp.release_threshold_tick =
        PROTECTION_NCM_ROCP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.rocp.is_pf_enabled = PROTECTION_NCM_ROCP_DEFAULT_PF_ENABLED;

    // OTP
    config->protection.otp.is_enabled = PROTECTION_NCM_OTP_DEFAULT_ENABLED;
    config->protection.otp.threshold_value = PROTECTION_NCM_OTP_DEFAULT_THRESHOLD;
    config->protection.otp.threshold_tick = PROTECTION_NCM_OTP_DEFAULT_THRESHOLD_TICK;
    config->protection.otp.is_release_enabled = PROTECTION_NCM_OTP_DEFAULT_RELEASE_ENABLED;
    config->protection.otp.release_threshold_value = PROTECTION_NCM_OTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.otp.release_threshold_tick =
        PROTECTION_NCM_OTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.otp.is_pf_enabled = PROTECTION_NCM_OTP_DEFAULT_PF_ENABLED;

    // COTP
    config->protection.cotp.is_enabled = PROTECTION_NCM_COTP_DEFAULT_ENABLED;
    config->protection.cotp.threshold_value = PROTECTION_NCM_COTP_DEFAULT_THRESHOLD;
    config->protection.cotp.threshold_tick = PROTECTION_NCM_COTP_DEFAULT_THRESHOLD_TICK;
    config->protection.cotp.is_release_enabled = PROTECTION_NCM_COTP_DEFAULT_RELEASE_ENABLED;
    config->protection.cotp.release_threshold_value = PROTECTION_NCM_COTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.cotp.release_threshold_tick =
        PROTECTION_NCM_COTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.cotp.is_pf_enabled = PROTECTION_NCM_COTP_DEFAULT_PF_ENABLED;

    // DOTP
    config->protection.dotp.is_enabled = PROTECTION_NCM_DOTP_DEFAULT_ENABLED;
    config->protection.dotp.threshold_value = PROTECTION_NCM_DOTP_DEFAULT_THRESHOLD;
    config->protection.dotp.threshold_tick = PROTECTION_NCM_DOTP_DEFAULT_THRESHOLD_TICK;
    config->protection.dotp.is_release_enabled = PROTECTION_NCM_DOTP_DEFAULT_RELEASE_ENABLED;
    config->protection.dotp.release_threshold_value = PROTECTION_NCM_DOTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.dotp.release_threshold_tick =
        PROTECTION_NCM_DOTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.dotp.is_pf_enabled = PROTECTION_NCM_DOTP_DEFAULT_PF_ENABLED;

    // ROTP
    config->protection.rotp.is_enabled = PROTECTION_NCM_ROTP_DEFAULT_ENABLED;
    config->protection.rotp.threshold_value = PROTECTION_NCM_ROTP_DEFAULT_THRESHOLD;
    config->protection.rotp.threshold_tick = PROTECTION_NCM_ROTP_DEFAULT_THRESHOLD_TICK;
    config->protection.rotp.is_release_enabled = PROTECTION_NCM_ROTP_DEFAULT_RELEASE_ENABLED;
    config->protection.rotp.release_threshold_value = PROTECTION_NCM_ROTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.rotp.release_threshold_tick =
        PROTECTION_NCM_ROTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.rotp.is_pf_enabled = PROTECTION_NCM_ROTP_DEFAULT_PF_ENABLED;

    // UTP
    config->protection.utp.is_enabled = PROTECTION_NCM_UTP_DEFAULT_ENABLED;
    config->protection.utp.threshold_value = PROTECTION_NCM_UTP_DEFAULT_THRESHOLD;
    config->protection.utp.threshold_tick = PROTECTION_NCM_UTP_DEFAULT_THRESHOLD_TICK;
    config->protection.utp.is_release_enabled = PROTECTION_NCM_UTP_DEFAULT_RELEASE_ENABLED;
    config->protection.utp.release_threshold_value = PROTECTION_NCM_UTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.utp.release_threshold_tick =
        PROTECTION_NCM_UTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.utp.is_pf_enabled = PROTECTION_NCM_UTP_DEFAULT_PF_ENABLED;

    // CUTP
    config->protection.cutp.is_enabled = PROTECTION_NCM_CUTP_DEFAULT_ENABLED;
    config->protection.cutp.threshold_value = PROTECTION_NCM_CUTP_DEFAULT_THRESHOLD;
    config->protection.cutp.threshold_tick = PROTECTION_NCM_CUTP_DEFAULT_THRESHOLD_TICK;
    config->protection.cutp.is_release_enabled = PROTECTION_NCM_CUTP_DEFAULT_RELEASE_ENABLED;
    config->protection.cutp.release_threshold_value = PROTECTION_NCM_CUTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.cutp.release_threshold_tick =
        PROTECTION_NCM_CUTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.cutp.is_pf_enabled = PROTECTION_NCM_CUTP_DEFAULT_PF_ENABLED;

    // DUTP
    config->protection.dutp.is_enabled = PROTECTION_NCM_DUTP_DEFAULT_ENABLED;
    config->protection.dutp.threshold_value = PROTECTION_NCM_DUTP_DEFAULT_THRESHOLD;
    config->protection.dutp.threshold_tick = PROTECTION_NCM_DUTP_DEFAULT_THRESHOLD_TICK;
    config->protection.dutp.is_release_enabled = PROTECTION_NCM_DUTP_DEFAULT_RELEASE_ENABLED;
    config->protection.dutp.release_threshold_value = PROTECTION_NCM_DUTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.dutp.release_threshold_tick =
        PROTECTION_NCM_DUTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.dutp.is_pf_enabled = PROTECTION_NCM_DUTP_DEFAULT_PF_ENABLED;

    // RUTP
    config->protection.rutp.is_enabled = PROTECTION_NCM_RUTP_DEFAULT_ENABLED;
    config->protection.rutp.threshold_value = PROTECTION_NCM_RUTP_DEFAULT_THRESHOLD;
    config->protection.rutp.threshold_tick = PROTECTION_NCM_RUTP_DEFAULT_THRESHOLD_TICK;
    config->protection.rutp.is_release_enabled = PROTECTION_NCM_RUTP_DEFAULT_RELEASE_ENABLED;
    config->protection.rutp.release_threshold_value = PROTECTION_NCM_RUTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.rutp.release_threshold_tick =
        PROTECTION_NCM_RUTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.rutp.is_pf_enabled = PROTECTION_NCM_RUTP_DEFAULT_PF_ENABLED;

    // ODVP
    config->protection.odvp.is_enabled = PROTECTION_NCM_ODVP_DEFAULT_ENABLED;
    config->protection.odvp.threshold_value = PROTECTION_NCM_ODVP_DEFAULT_THRESHOLD;
    config->protection.odvp.threshold_tick = PROTECTION_NCM_ODVP_DEFAULT_THRESHOLD_TICK;
    config->protection.odvp.is_release_enabled = PROTECTION_NCM_ODVP_DEFAULT_RELEASE_ENABLED;
    config->protection.odvp.release_threshold_value = PROTECTION_NCM_ODVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.odvp.release_threshold_tick =
        PROTECTION_NCM_ODVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.odvp.is_pf_enabled = PROTECTION_NCM_ODVP_DEFAULT_PF_ENABLED;

    // CODVP
    config->protection.codvp.is_enabled = PROTECTION_NCM_CODVP_DEFAULT_ENABLED;
    config->protection.codvp.threshold_value = PROTECTION_NCM_CODVP_DEFAULT_THRESHOLD;
    config->protection.codvp.threshold_tick = PROTECTION_NCM_CODVP_DEFAULT_THRESHOLD_TICK;
    config->protection.codvp.is_release_enabled = PROTECTION_NCM_CODVP_DEFAULT_RELEASE_ENABLED;
    config->protection.codvp.release_threshold_value =
        PROTECTION_NCM_CODVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.codvp.release_threshold_tick =
        PROTECTION_NCM_CODVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.codvp.is_pf_enabled = PROTECTION_NCM_CODVP_DEFAULT_PF_ENABLED;

    // DODVP
    config->protection.dodvp.is_enabled = PROTECTION_NCM_DODVP_DEFAULT_ENABLED;
    config->protection.dodvp.threshold_value = PROTECTION_NCM_DODVP_DEFAULT_THRESHOLD;
    config->protection.dodvp.threshold_tick = PROTECTION_NCM_DODVP_DEFAULT_THRESHOLD_TICK;
    config->protection.dodvp.is_release_enabled = PROTECTION_NCM_DODVP_DEFAULT_RELEASE_ENABLED;
    config->protection.dodvp.release_threshold_value =
        PROTECTION_NCM_DODVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.dodvp.release_threshold_tick =
        PROTECTION_NCM_DODVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.dodvp.is_pf_enabled = PROTECTION_NCM_DODVP_DEFAULT_PF_ENABLED;

    // RODVP
    config->protection.rodvp.is_enabled = PROTECTION_NCM_RODVP_DEFAULT_ENABLED;
    config->protection.rodvp.threshold_value = PROTECTION_NCM_RODVP_DEFAULT_THRESHOLD;
    config->protection.rodvp.threshold_tick = PROTECTION_NCM_RODVP_DEFAULT_THRESHOLD_TICK;
    config->protection.rodvp.is_release_enabled = PROTECTION_NCM_RODVP_DEFAULT_RELEASE_ENABLED;
    config->protection.rodvp.release_threshold_value =
        PROTECTION_NCM_RODVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.rodvp.release_threshold_tick =
        PROTECTION_NCM_RODVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.rodvp.is_pf_enabled = PROTECTION_NCM_RODVP_DEFAULT_PF_ENABLED;

    // ODTP
    config->protection.odtp.is_enabled = PROTECTION_NCM_ODTP_DEFAULT_ENABLED;
    config->protection.odtp.threshold_value = PROTECTION_NCM_ODTP_DEFAULT_THRESHOLD;
    config->protection.odtp.threshold_tick = PROTECTION_NCM_ODTP_DEFAULT_THRESHOLD_TICK;
    config->protection.odtp.is_release_enabled = PROTECTION_NCM_ODTP_DEFAULT_RELEASE_ENABLED;
    config->protection.odtp.release_threshold_value = PROTECTION_NCM_ODTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.odtp.release_threshold_tick =
        PROTECTION_NCM_ODTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.odtp.is_pf_enabled = PROTECTION_NCM_ODTP_DEFAULT_PF_ENABLED;

    // OPP
    config->protection.opp.is_enabled = PROTECTION_NCM_OPP_DEFAULT_ENABLED;
    config->protection.opp.threshold_value = PROTECTION_NCM_OPP_DEFAULT_THRESHOLD;
    config->protection.opp.threshold_tick = PROTECTION_NCM_OPP_DEFAULT_THRESHOLD_TICK;
    config->protection.opp.is_release_enabled = PROTECTION_NCM_OPP_DEFAULT_RELEASE_ENABLED;
    config->protection.opp.release_threshold_value = PROTECTION_NCM_OPP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.opp.release_threshold_tick =
        PROTECTION_NCM_OPP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.opp.is_pf_enabled = PROTECTION_NCM_OPP_DEFAULT_PF_ENABLED;

    // UPP
    config->protection.upp.is_enabled = PROTECTION_NCM_UPP_DEFAULT_ENABLED;
    config->protection.upp.threshold_value = PROTECTION_NCM_UPP_DEFAULT_THRESHOLD;
    config->protection.upp.threshold_tick = PROTECTION_NCM_UPP_DEFAULT_THRESHOLD_TICK;
    config->protection.upp.is_release_enabled = PROTECTION_NCM_UPP_DEFAULT_RELEASE_ENABLED;
    config->protection.upp.release_threshold_value = PROTECTION_NCM_UPP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.upp.release_threshold_tick =
        PROTECTION_NCM_UPP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.upp.is_pf_enabled = PROTECTION_NCM_UPP_DEFAULT_PF_ENABLED;

    // misc
    config->misc.protection_extension.odvp_threshold = PROTECTION_NCM_CODVP_CHECK_THRESHOLD;
    config->misc.bdi.add_value = 100;
}

// Do not use this function directly, use BmsConfig_CreateDefault() instead
static inline void _BmsConfig_SetLfp(BmsConfig* config) {
    // pack
#if PACK_LFP_DEFAULT_MODEL == 0
    config->pack.cell_model = BMS_CELL_MODEL_EVE_LF105;
#elif PACK_LFP_DEFAULT_MODEL == 1
    config->pack.cell_model = BMS_CELL_MODEL_EVE_LF150;
#else
    config->pack.cell_model = BMS_CELL_MODEL_EVE_LF105;
#endif
    config->pack.parallel_count = PACK_LFP_DEFAULT_PARALLEL_COUNT;
    config->pack.serial_count = PACK_LFP_DEFAULT_SERIAL_COUNT;

    // soc
    config->soc.ocv_calibration_tick = SOC_LFP_DEFAULT_OCV_CALIBRATION_TICK;
    config->soc.capacity_calibration_tick = SOC_LFP_DEFAULT_CAPACITY_CALIBRATION_TICK;

    // charger
    config->charger.cv_voltage = CHARGER_LFP_DEFAULT_CV_VOLTAGE;
    config->charger.recharge_start_voltage = CHARGER_LFP_DEFAULT_RECHARGE_START_VOLTAGE;

    // balancing
    config->balancing.threshold_dv = BALANCING_DEFAULT_LFP_THRESHOLD_DV;
    config->balancing.threshold_bank_voltage = BALANCING_DEFAULT_LFP_THRESHOLD_BANK_VOLTAGE;
    config->balancing.release_threshold_dv = BALANCING_DEFAULT_LFP_RELEASE_THRESHOLD_DV;

    // OVP
    config->protection.ovp.is_enabled = PROTECTION_LFP_OVP_DEFAULT_ENABLED;
    config->protection.ovp.threshold_value = PROTECTION_LFP_OVP_DEFAULT_THRESHOLD;
    config->protection.ovp.threshold_tick = PROTECTION_LFP_OVP_DEFAULT_THRESHOLD_TICK;
    config->protection.ovp.is_release_enabled = PROTECTION_LFP_OVP_DEFAULT_RELEASE_ENABLED;
    config->protection.ovp.release_threshold_value = PROTECTION_LFP_OVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.ovp.release_threshold_tick =
        PROTECTION_LFP_OVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.ovp.is_pf_enabled = PROTECTION_LFP_OVP_DEFAULT_PF_ENABLED;

    // UVP
    config->protection.uvp.is_enabled = PROTECTION_LFP_UVP_DEFAULT_ENABLED;
    config->protection.uvp.threshold_value = PROTECTION_LFP_UVP_DEFAULT_THRESHOLD;
    config->protection.uvp.threshold_tick = PROTECTION_LFP_UVP_DEFAULT_THRESHOLD_TICK;
    config->protection.uvp.is_release_enabled = PROTECTION_LFP_UVP_DEFAULT_RELEASE_ENABLED;
    config->protection.uvp.release_threshold_value = PROTECTION_LFP_UVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.uvp.release_threshold_tick =
        PROTECTION_LFP_UVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.uvp.is_pf_enabled = PROTECTION_LFP_UVP_DEFAULT_PF_ENABLED;

    // COCP
    config->protection.cocp.is_enabled = PROTECTION_LFP_COCP_DEFAULT_ENABLED;
    config->protection.cocp.threshold_value = PROTECTION_LFP_COCP_DEFAULT_THRESHOLD;
    config->protection.cocp.threshold_tick = PROTECTION_LFP_COCP_DEFAULT_THRESHOLD_TICK;
    config->protection.cocp.is_release_enabled = PROTECTION_LFP_COCP_DEFAULT_RELEASE_ENABLED;
    config->protection.cocp.release_threshold_value = PROTECTION_LFP_COCP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.cocp.release_threshold_tick =
        PROTECTION_LFP_COCP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.cocp.is_pf_enabled = PROTECTION_LFP_COCP_DEFAULT_PF_ENABLED;

    // DOCP
    config->protection.docp.is_enabled = PROTECTION_LFP_DOCP_DEFAULT_ENABLED;
    config->protection.docp.threshold_value = PROTECTION_LFP_DOCP_DEFAULT_THRESHOLD;
    config->protection.docp.threshold_tick = PROTECTION_LFP_DOCP_DEFAULT_THRESHOLD_TICK;
    config->protection.docp.is_release_enabled = PROTECTION_LFP_DOCP_DEFAULT_RELEASE_ENABLED;
    config->protection.docp.release_threshold_value = PROTECTION_LFP_DOCP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.docp.release_threshold_tick =
        PROTECTION_LFP_DOCP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.docp.is_pf_enabled = PROTECTION_LFP_DOCP_DEFAULT_PF_ENABLED;

    // ROCP
    config->protection.rocp.is_enabled = PROTECTION_LFP_ROCP_DEFAULT_ENABLED;
    config->protection.rocp.threshold_value = PROTECTION_LFP_ROCP_DEFAULT_THRESHOLD;
    config->protection.rocp.threshold_tick = PROTECTION_LFP_ROCP_DEFAULT_THRESHOLD_TICK;
    config->protection.rocp.is_release_enabled = PROTECTION_LFP_ROCP_DEFAULT_RELEASE_ENABLED;
    config->protection.rocp.release_threshold_value = PROTECTION_LFP_ROCP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.rocp.release_threshold_tick =
        PROTECTION_LFP_ROCP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.rocp.is_pf_enabled = PROTECTION_LFP_ROCP_DEFAULT_PF_ENABLED;

    // OTP
    config->protection.otp.is_enabled = PROTECTION_LFP_OTP_DEFAULT_ENABLED;
    config->protection.otp.threshold_value = PROTECTION_LFP_OTP_DEFAULT_THRESHOLD;
    config->protection.otp.threshold_tick = PROTECTION_LFP_OTP_DEFAULT_THRESHOLD_TICK;
    config->protection.otp.is_release_enabled = PROTECTION_LFP_OTP_DEFAULT_RELEASE_ENABLED;
    config->protection.otp.release_threshold_value = PROTECTION_LFP_OTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.otp.release_threshold_tick =
        PROTECTION_LFP_OTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.otp.is_pf_enabled = PROTECTION_LFP_OTP_DEFAULT_PF_ENABLED;

    // COTP
    config->protection.cotp.is_enabled = PROTECTION_LFP_COTP_DEFAULT_ENABLED;
    config->protection.cotp.threshold_value = PROTECTION_LFP_COTP_DEFAULT_THRESHOLD;
    config->protection.cotp.threshold_tick = PROTECTION_LFP_COTP_DEFAULT_THRESHOLD_TICK;
    config->protection.cotp.is_release_enabled = PROTECTION_LFP_COTP_DEFAULT_RELEASE_ENABLED;
    config->protection.cotp.release_threshold_value = PROTECTION_LFP_COTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.cotp.release_threshold_tick =
        PROTECTION_LFP_COTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.cotp.is_pf_enabled = PROTECTION_LFP_COTP_DEFAULT_PF_ENABLED;

    // DOTP
    config->protection.dotp.is_enabled = PROTECTION_LFP_DOTP_DEFAULT_ENABLED;
    config->protection.dotp.threshold_value = PROTECTION_LFP_DOTP_DEFAULT_THRESHOLD;
    config->protection.dotp.threshold_tick = PROTECTION_LFP_DOTP_DEFAULT_THRESHOLD_TICK;
    config->protection.dotp.is_release_enabled = PROTECTION_LFP_DOTP_DEFAULT_RELEASE_ENABLED;
    config->protection.dotp.release_threshold_value = PROTECTION_LFP_DOTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.dotp.release_threshold_tick =
        PROTECTION_LFP_DOTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.dotp.is_pf_enabled = PROTECTION_LFP_DOTP_DEFAULT_PF_ENABLED;

    // ROTP
    config->protection.rotp.is_enabled = PROTECTION_LFP_ROTP_DEFAULT_ENABLED;
    config->protection.rotp.threshold_value = PROTECTION_LFP_ROTP_DEFAULT_THRESHOLD;
    config->protection.rotp.threshold_tick = PROTECTION_LFP_ROTP_DEFAULT_THRESHOLD_TICK;
    config->protection.rotp.is_release_enabled = PROTECTION_LFP_ROTP_DEFAULT_RELEASE_ENABLED;
    config->protection.rotp.release_threshold_value = PROTECTION_LFP_ROTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.rotp.release_threshold_tick =
        PROTECTION_LFP_ROTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.rotp.is_pf_enabled = PROTECTION_LFP_ROTP_DEFAULT_PF_ENABLED;

    // UTP
    config->protection.utp.is_enabled = PROTECTION_LFP_UTP_DEFAULT_ENABLED;
    config->protection.utp.threshold_value = PROTECTION_LFP_UTP_DEFAULT_THRESHOLD;
    config->protection.utp.threshold_tick = PROTECTION_LFP_UTP_DEFAULT_THRESHOLD_TICK;
    config->protection.utp.is_release_enabled = PROTECTION_LFP_UTP_DEFAULT_RELEASE_ENABLED;
    config->protection.utp.release_threshold_value = PROTECTION_LFP_UTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.utp.release_threshold_tick =
        PROTECTION_LFP_UTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.utp.is_pf_enabled = PROTECTION_LFP_UTP_DEFAULT_PF_ENABLED;

    // CUTP
    config->protection.cutp.is_enabled = PROTECTION_LFP_CUTP_DEFAULT_ENABLED;
    config->protection.cutp.threshold_value = PROTECTION_LFP_CUTP_DEFAULT_THRESHOLD;
    config->protection.cutp.threshold_tick = PROTECTION_LFP_CUTP_DEFAULT_THRESHOLD_TICK;
    config->protection.cutp.is_release_enabled = PROTECTION_LFP_CUTP_DEFAULT_RELEASE_ENABLED;
    config->protection.cutp.release_threshold_value = PROTECTION_LFP_CUTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.cutp.release_threshold_tick =
        PROTECTION_LFP_CUTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.cutp.is_pf_enabled = PROTECTION_LFP_CUTP_DEFAULT_PF_ENABLED;

    // DUTP
    config->protection.dutp.is_enabled = PROTECTION_LFP_DUTP_DEFAULT_ENABLED;
    config->protection.dutp.threshold_value = PROTECTION_LFP_DUTP_DEFAULT_THRESHOLD;
    config->protection.dutp.threshold_tick = PROTECTION_LFP_DUTP_DEFAULT_THRESHOLD_TICK;
    config->protection.dutp.is_release_enabled = PROTECTION_LFP_DUTP_DEFAULT_RELEASE_ENABLED;
    config->protection.dutp.release_threshold_value = PROTECTION_LFP_DUTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.dutp.release_threshold_tick =
        PROTECTION_LFP_DUTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.dutp.is_pf_enabled = PROTECTION_LFP_DUTP_DEFAULT_PF_ENABLED;

    // RUTP
    config->protection.rutp.is_enabled = PROTECTION_LFP_RUTP_DEFAULT_ENABLED;
    config->protection.rutp.threshold_value = PROTECTION_LFP_RUTP_DEFAULT_THRESHOLD;
    config->protection.rutp.threshold_tick = PROTECTION_LFP_RUTP_DEFAULT_THRESHOLD_TICK;
    config->protection.rutp.is_release_enabled = PROTECTION_LFP_RUTP_DEFAULT_RELEASE_ENABLED;
    config->protection.rutp.release_threshold_value = PROTECTION_LFP_RUTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.rutp.release_threshold_tick =
        PROTECTION_LFP_RUTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.rutp.is_pf_enabled = PROTECTION_LFP_RUTP_DEFAULT_PF_ENABLED;

    // ODVP
    config->protection.odvp.is_enabled = PROTECTION_LFP_ODVP_DEFAULT_ENABLED;
    config->protection.odvp.threshold_value = PROTECTION_LFP_ODVP_DEFAULT_THRESHOLD;
    config->protection.odvp.threshold_tick = PROTECTION_LFP_ODVP_DEFAULT_THRESHOLD_TICK;
    config->protection.odvp.is_release_enabled = PROTECTION_LFP_ODVP_DEFAULT_RELEASE_ENABLED;
    config->protection.odvp.release_threshold_value = PROTECTION_LFP_ODVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.odvp.release_threshold_tick =
        PROTECTION_LFP_ODVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.odvp.is_pf_enabled = PROTECTION_LFP_ODVP_DEFAULT_PF_ENABLED;

    // CODVP
    config->protection.codvp.is_enabled = PROTECTION_LFP_CODVP_DEFAULT_ENABLED;
    config->protection.codvp.threshold_value = PROTECTION_LFP_CODVP_DEFAULT_THRESHOLD;
    config->protection.codvp.threshold_tick = PROTECTION_LFP_CODVP_DEFAULT_THRESHOLD_TICK;
    config->protection.codvp.is_release_enabled = PROTECTION_LFP_CODVP_DEFAULT_RELEASE_ENABLED;
    config->protection.codvp.release_threshold_value =
        PROTECTION_LFP_CODVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.codvp.release_threshold_tick =
        PROTECTION_LFP_CODVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.codvp.is_pf_enabled = PROTECTION_LFP_CODVP_DEFAULT_PF_ENABLED;

    // DODVP
    config->protection.dodvp.is_enabled = PROTECTION_LFP_DODVP_DEFAULT_ENABLED;
    config->protection.dodvp.threshold_value = PROTECTION_LFP_DODVP_DEFAULT_THRESHOLD;
    config->protection.dodvp.threshold_tick = PROTECTION_LFP_DODVP_DEFAULT_THRESHOLD_TICK;
    config->protection.dodvp.is_release_enabled = PROTECTION_LFP_DODVP_DEFAULT_RELEASE_ENABLED;
    config->protection.dodvp.release_threshold_value =
        PROTECTION_LFP_DODVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.dodvp.release_threshold_tick =
        PROTECTION_LFP_DODVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.dodvp.is_pf_enabled = PROTECTION_LFP_DODVP_DEFAULT_PF_ENABLED;

    // RODVP
    config->protection.rodvp.is_enabled = PROTECTION_LFP_RODVP_DEFAULT_ENABLED;
    config->protection.rodvp.threshold_value = PROTECTION_LFP_RODVP_DEFAULT_THRESHOLD;
    config->protection.rodvp.threshold_tick = PROTECTION_LFP_RODVP_DEFAULT_THRESHOLD_TICK;
    config->protection.rodvp.is_release_enabled = PROTECTION_LFP_RODVP_DEFAULT_RELEASE_ENABLED;
    config->protection.rodvp.release_threshold_value =
        PROTECTION_LFP_RODVP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.rodvp.release_threshold_tick =
        PROTECTION_LFP_RODVP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.rodvp.is_pf_enabled = PROTECTION_LFP_RODVP_DEFAULT_PF_ENABLED;

    // ODTP
    config->protection.odtp.is_enabled = PROTECTION_LFP_ODTP_DEFAULT_ENABLED;
    config->protection.odtp.threshold_value = PROTECTION_LFP_ODTP_DEFAULT_THRESHOLD;
    config->protection.odtp.threshold_tick = PROTECTION_LFP_ODTP_DEFAULT_THRESHOLD_TICK;
    config->protection.odtp.is_release_enabled = PROTECTION_LFP_ODTP_DEFAULT_RELEASE_ENABLED;
    config->protection.odtp.release_threshold_value = PROTECTION_LFP_ODTP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.odtp.release_threshold_tick =
        PROTECTION_LFP_ODTP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.odtp.is_pf_enabled = PROTECTION_LFP_ODTP_DEFAULT_PF_ENABLED;

    // OPP
    config->protection.opp.is_enabled = PROTECTION_LFP_OPP_DEFAULT_ENABLED;
    config->protection.opp.threshold_value = PROTECTION_LFP_OPP_DEFAULT_THRESHOLD;
    config->protection.opp.threshold_tick = PROTECTION_LFP_OPP_DEFAULT_THRESHOLD_TICK;
    config->protection.opp.is_release_enabled = PROTECTION_LFP_OPP_DEFAULT_RELEASE_ENABLED;
    config->protection.opp.release_threshold_value = PROTECTION_LFP_OPP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.opp.release_threshold_tick =
        PROTECTION_LFP_OPP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.opp.is_pf_enabled = PROTECTION_LFP_OPP_DEFAULT_PF_ENABLED;

    // UPP
    config->protection.upp.is_enabled = PROTECTION_LFP_UPP_DEFAULT_ENABLED;
    config->protection.upp.threshold_value = PROTECTION_LFP_UPP_DEFAULT_THRESHOLD;
    config->protection.upp.threshold_tick = PROTECTION_LFP_UPP_DEFAULT_THRESHOLD_TICK;
    config->protection.upp.is_release_enabled = PROTECTION_LFP_UPP_DEFAULT_RELEASE_ENABLED;
    config->protection.upp.release_threshold_value = PROTECTION_LFP_UPP_DEFAULT_RELEASE_THRESHOLD;
    config->protection.upp.release_threshold_tick =
        PROTECTION_LFP_UPP_DEFAULT_RELEASE_THRESHOLD_TICK;
    config->protection.upp.is_pf_enabled = PROTECTION_LFP_UPP_DEFAULT_PF_ENABLED;

    // misc
    config->misc.protection_extension.odvp_threshold = PROTECTION_LFP_CODVP_CHECK_THRESHOLD;
    config->misc.bdi.add_value = 0;
}

static inline BmsConfig BmsConfig_CreateDefault(void) {
    BmsConfig config;
    _BmsConfig_SetCommon(&config);

#if PACK_DEFAULT_CELL_MODEL_TYPE == 0  // NCM
    _BmsConfig_SetNcm(&config);

#elif PACK_DEFAULT_CELL_MODEL_TYPE == 1  // LFP
    _BmsConfig_SetLfp(&config);

#endif

    return config;
}

#endif
