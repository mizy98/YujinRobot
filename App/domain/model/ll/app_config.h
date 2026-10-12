#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// App information
/*==========================================================================*/
#define APP_VENDOR_NAME "SMI ES"
#define APP_PROJECT_NAME "BMS F1"
#define APP_PROJECT_VERSION_MAJOR 1
#define APP_PROJECT_VERSION_MINOR 0
#define APP_PROJECT_VERSION_PATCH 0
#define APP_PROJECT_VERSION_NAME        \
    TOSTRING(APP_PROJECT_VERSION_MAJOR) \
    "." TOSTRING(APP_PROJECT_VERSION_MINOR) "." TOSTRING(APP_PROJECT_VERSION_PATCH)
#define APP_VENDOR_NAME_MAX_LENGTH (32u)
#define APP_PROJECT_NAME_MAX_LENGTH (32u)
#define APP_PRODUCT_NAME_MAX_LENGTH (32u)
#define APP_DEFAULT_PRODUCT_NAME "HGP2"
#define APP_CONFIG_VERSION (11u)
#define APP_DEVICE_ID (5u)
/*==========================================================================*/

// Pack
/*==========================================================================*/
#define PACK_MAX_SERIAL_COUNT (16u)
#define PACK_MAX_PARALLEL_COUNT (255u)
#define PACK_DEFAULT_CELL_MODEL_TYPE (0u)  // 0: NCM / LiPo, 1: LFP

#define PACK_NCM_DEFAULT_MODEL \
    (0)  // 0: LGES INR21700M50LT
         // 1: SKON E600
#define PACK_NCM_DEFAULT_SERIAL_COUNT (14u)
#define PACK_NCM_DEFAULT_PARALLEL_COUNT (36u)

#define PACK_LFP_DEFAULT_MODEL \
    (0)  // 0: EVE LF105
         // 1: EVE LF150
#define PACK_LFP_DEFAULT_SERIAL_COUNT (16u)
#define PACK_LFP_DEFAULT_PARALLEL_COUNT (1u)

#if PACK_DEFAULT_CELL_MODEL_TYPE > (1u)
#error "Invalid PACK_DEFAULT_CELL_MODEL_TYPE value"
#endif
/*==========================================================================*/

// Voltage
/*==========================================================================*/
#define VOLTAGE_READ_TICK (250u)  // ms
#define VOLTAGE_MAX_COUNT (16u)
/*==========================================================================*/

// Current
/*==========================================================================*/
#define CURRENT_READ_TICK (100u)                             // ms
#define CURRENT_RESOLUTION_VOLTAGE (0.005f)                  // V
#define CURRENT_CALIBRATION_DEFAULT_TICK (1000u * 60u * 5u)  // ms
#define CURRENT_DIRECTION (1u)                               // 0: backward, 1: forward
#define CURRENT_EMA_ALPHA (100u)  // 0.001 (0 ~ 1000), fixed-point with 3 decimal places
/*==========================================================================*/

// Temperature
/*==========================================================================*/
#define TEMPERATURE_READ_TICK (1000u)
#define TEMPERATURE_DEFAULT_PACK_COUNT (3u)
#define TEMPERATURE_DEFAULT_BOARD_COUNT (1u)
#define TEMPERATURE_DEFAULT_BALANCER_COUNT (1u)
#define TEMPERATURE_DEFAULT_CONTACTOR_COUNT (1u)
#define TEMPERATURE_MAX_PACK_COUNT (3u)
#define TEMPERATURE_MAX_BOARD_COUNT (2u)
#define TEMPERATURE_MAX_BALANCER_COUNT (1u)
#define TEMPERATURE_MAX_CONTACTOR_COUNT (1u)
#define TEMPERATURE_SERIAL_RESISTOR (10.0f)  // ㏀
/*==========================================================================*/

// State
/*==========================================================================*/
#define STATE_DEFAULT_CHARGE_PARSE_MODE \
    (2u)  // 0: BMS_CHARGE_PARSE_MODE_CC,
          // 1: BMS_CHARGE_PARSE_MODE_SIGNAL
          // 2: BMS_CHARGE_PARSE_MODE_TIME

#define STATE_DEFAULT_CHARGE_THRESHOLD_CURRENT (30)                  // 0.1A
#define STATE_DEFAULT_DISCHARGE_THRESHOLD_CURRENT (-30)              // 0.1A
#define STATE_DEFAULT_CHARGE_THRESHOLD_TICK (1000u * 60u * 5u)       // ms
#define STATE_CC_SAMPLE_TICK (100u)                                  // ms
#define STATE_CC_THRESHOLD_TICK (1000u * 60u * 2u)                   // ms
#define STATE_CC_STABLE_THRESHOLD_CURRENT (25)                       // 0.1A
#define STATE_DEFAULT_SLEEP_THRESHOLD_TICK (1000u * 60u * 60u * 2u)  // ms
#define STATE_DEFAULT_SLEEP_THRESHOLD_SOC (300u)                     // 0.1%
#define STATE_DEFAULT_SLEEP_THRESHOLD_VOLTAGE (3200u)                // mV
#define STATE_SLEEP_WAKEUP_SIGNAL_THRESHOLD_TICK (3000u)             // ms
/*==========================================================================*/

// SOC
/*==========================================================================*/
#define SOC_NCM_DEFAULT_OCV_CALIBRATION_TICK (1000u * 60u * 5u)        // ms
#define SOC_NCM_DEFAULT_CAPACITY_CALIBRATION_TICK (1000u * 60u * 5u)   // ms
#define SOC_LFP_DEFAULT_OCV_CALIBRATION_TICK (1000u * 60u * 10u)       // ms
#define SOC_LFP_DEFAULT_CAPACITY_CALIBRATION_TICK (1000u * 60u * 10u)  // ms
#define SOC_DEFAULT_TRACK_UNIT (1u)                                    // 0.1%
#define SOC_DEFAULT_TRACK_TICK (1000u * 60u)                           // ms
/*==========================================================================*/

// Contactor
/*==========================================================================*/
#define CONTACTOR_DEFAULT_MODE (3u)  // @ref BmsContactorMode

#define CONTACTOR_MAIN_DEFAULT_CHECK_TICK (0u)       // ms, 0 means no check
#define CONTACTOR_CHARGE_DEFAULT_CHECK_TICK (0u)     // ms, 0 means no check
#define CONTACTOR_DISCHARGE_DEFAULT_CHECK_TICK (0u)  // ms, 0 means no check
#define CONTACTOR_PRECHARGE_DEFAULT_CHECK_TICK (0u)  // ms, 0 means no check

#define CONTACTOR_MAIN_DEFAULT_PRECHARGE_TICK (1500u)    // ms, 0 means no check
#define CONTACTOR_CHARGE_DEFAULT_PRECHARGE_TICK (0u)     // ms, 0 means no check
#define CONTACTOR_DISCHARGE_DEFAULT_PRECHARGE_TICK (0u)  // ms, 0 means no check

#define CONTACTOR_EXTERNAL_DEFAULT_ENABLED (0u)
#define CONTACTOR_EXTERNAL_DEFAULT_THRESHOLD_TICK (1000u * 3u)  // ms
/*==========================================================================*/

// Balancing
/*==========================================================================*/
#define BALANCING_DEFAULT_ENABLED (1u)
#define BALANCING_DEFAULT_NCM_THRESHOLD_DV (50u)                // mV
#define BALANCING_DEFAULT_LFP_THRESHOLD_DV (50u)                // mV
#define BALANCING_DEFAULT_NCM_THRESHOLD_BANK_VOLTAGE (3900u)    // mV
#define BALANCING_DEFAULT_LFP_THRESHOLD_BANK_VOLTAGE (3500u)    // mV
#define BALANCING_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define BALANCING_DEFAULT_NCM_RELEASE_THRESHOLD_DV (30u)        // mV
#define BALANCING_DEFAULT_LFP_RELEASE_THRESHOLD_DV (30u)        // mV
#define BALANCING_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
/*==========================================================================*/

// Fan
/*==========================================================================*/
#define FAN_MAX_COUNT (8u)
#define FAN_DEFAULT_ENABLED (1u)
#define FAN_DEFAULT_THRESHOLD_SOC (300u)                  // 0.1%
#define FAN_DEFAULT_THRESHOLD_VALUE (450)                 // 0.1℃
#define FAN_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define FAN_DEFAULT_RELEASE_THRESHOLD_VALUE (400)         // 0.1℃
#define FAN_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
/*==========================================================================*/

// Heater
/*==========================================================================*/
#define HEATER_MAX_COUNT (8u)
#define HEATER_DEFAULT_ENABLED (0u)
#define HEATER_DEFAULT_THRESHOLD_SOC (300u)                  // 0.1%
#define HEATER_DEFAULT_THRESHOLD_VALUE (0)                   // 0.1℃
#define HEATER_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define HEATER_DEFAULT_RELEASE_THRESHOLD_VALUE (50)          // 0.1℃
#define HEATER_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
/*==========================================================================*/

// BDI
/*==========================================================================*/
#define BDI_DEFAULT_TYPE ((1 << 1u))  // @ref BmsBdiType
/*==========================================================================*/

// Charger
/*==========================================================================*/
#define CHARGER_DEFAULT_MODE (0u)          // @ref BmsChargerMode
#define CHARGER_DEFAULT_CC_CURRENT (150u)  // 0.1A
#define CHARGER_NCM_DEFAULT_CV_VOLTAGE ((4000u * PACK_NCM_DEFAULT_SERIAL_COUNT) / 100u)  // 0.1V
#define CHARGER_LFP_DEFAULT_CV_VOLTAGE ((3400u * PACK_LFP_DEFAULT_SERIAL_COUNT) / 100u)  // 0.1V
#define CHARGER_DEFAULT_CV_CUTOFF_CURRENT (20u)                                          // 0.1A
#define CHARGER_NCM_DEFAULT_RECHARGE_START_VOLTAGE \
    ((3800u * PACK_NCM_DEFAULT_SERIAL_COUNT) / 100u)  // 0.1V
#define CHARGER_LFP_DEFAULT_RECHARGE_START_VOLTAGE \
    ((3350u * PACK_LFP_DEFAULT_SERIAL_COUNT) / 100u)  // 0.1V
/*==========================================================================*/

// Data Log
/*==========================================================================*/
#define DATA_LOG_MAX_COUNT (31u)
#define DATA_LOG_MAX_PAGE_SIZE (2048u)
#define DATA_LOG_DEFAULT_TIME_ZONE_OFFSET_HOURS (9u)  // hours (UTC+9 KST)
/*==========================================================================*/

// Event Log
/*==========================================================================*/
#define EVENT_LOG_MAX_COUNT (31u)
#define EVENT_LOG_MAX_PAGE_SIZE (2048u)
#define EVENT_LOG_DEFAULT_TIME_ZONE_OFFSET_HOURS (9u)  // hours (UTC+9 KST)
/*==========================================================================*/

// CAN
/*==========================================================================*/
#define CAN_UI_COMM_ID (0x47F)
#define CAN_UI_COMM_IDE (0u)                 // 0: standard, 1: extended
#define CAN_CHANNEL_1_DEFAULT_BAUDRATE (1u)  // 0: 250kb/s, 1: 500kb/s
#define CAN_CHANNEL_2_DEFAULT_BAUDRATE (1u)  // 0: 250kb/s, 1: 500kb/s
#define CAN_ID_ALWAYZ_CLUSTER (0x10F8159E)
/*==========================================================================*/

// RTC
/*==========================================================================*/
#define RTC_OFFSET_YEAR (2025u)
/*==========================================================================*/

// Protection
/*==========================================================================*/
// Common
#define PROTECTION_DEFAULT_UVP_DELAY_TICK (1000u * 60u * 1u)  // ms

#define PROTECTION_CHECK_TICK (100u)  // ms

#define PROTECTION_DEFAULT_THRESHOLD_TICK (3000u)          // ms
#define PROTECTION_DEFAULT_RELEASE_THRESHOLD_TICK (3000u)  // ms

// NCM OVP
#define PROTECTION_NCM_OVP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_OVP_DEFAULT_THRESHOLD (4200)                      // mV
#define PROTECTION_NCM_OVP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_OVP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_OVP_DEFAULT_RELEASE_THRESHOLD (4100)              // mV
#define PROTECTION_NCM_OVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_OVP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM UVP
#define PROTECTION_NCM_UVP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_UVP_DEFAULT_THRESHOLD (3100)                      // mV
#define PROTECTION_NCM_UVP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_UVP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_UVP_DEFAULT_RELEASE_THRESHOLD (3200)              // mV
#define PROTECTION_NCM_UVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_UVP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM COCP
#define PROTECTION_NCM_COCP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_COCP_DEFAULT_THRESHOLD (1300)                      // 0.1A
#define PROTECTION_NCM_COCP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_COCP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_COCP_DEFAULT_RELEASE_THRESHOLD (30)                // 0.1A
#define PROTECTION_NCM_COCP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_COCP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM DOCP
#define PROTECTION_NCM_DOCP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_DOCP_DEFAULT_THRESHOLD (-3600)                     // 0.1A
#define PROTECTION_NCM_DOCP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_DOCP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_DOCP_DEFAULT_RELEASE_THRESHOLD (-30)               // 0.1A
#define PROTECTION_NCM_DOCP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_DOCP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM ROCP
#define PROTECTION_NCM_ROCP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_ROCP_DEFAULT_THRESHOLD (3600)                      // 0.1A
#define PROTECTION_NCM_ROCP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_ROCP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_ROCP_DEFAULT_RELEASE_THRESHOLD (30)                // 0.1A
#define PROTECTION_NCM_ROCP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_ROCP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM OTP
#define PROTECTION_NCM_OTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_OTP_DEFAULT_THRESHOLD (650)                       // 0.1℃
#define PROTECTION_NCM_OTP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_OTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_OTP_DEFAULT_RELEASE_THRESHOLD (500)               // 0.1℃
#define PROTECTION_NCM_OTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_OTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM COTP
#define PROTECTION_NCM_COTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_COTP_DEFAULT_THRESHOLD (500)                       // 0.1℃
#define PROTECTION_NCM_COTP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_COTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_COTP_DEFAULT_RELEASE_THRESHOLD (500)               // 0.1℃
#define PROTECTION_NCM_COTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_COTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM DOTP
#define PROTECTION_NCM_DOTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_DOTP_DEFAULT_THRESHOLD (650)                       // 0.1℃
#define PROTECTION_NCM_DOTP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_DOTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_DOTP_DEFAULT_RELEASE_THRESHOLD (600)               // 0.1℃
#define PROTECTION_NCM_DOTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_DOTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM ROTP
#define PROTECTION_NCM_ROTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_ROTP_DEFAULT_THRESHOLD (650)                       // 0.1℃
#define PROTECTION_NCM_ROTP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_ROTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_ROTP_DEFAULT_RELEASE_THRESHOLD (500)               // 0.1℃
#define PROTECTION_NCM_ROTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_ROTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM UTP
#define PROTECTION_NCM_UTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_UTP_DEFAULT_THRESHOLD (-200)                      // 0.1℃
#define PROTECTION_NCM_UTP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_UTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_UTP_DEFAULT_RELEASE_THRESHOLD (-50)               // 0.1℃
#define PROTECTION_NCM_UTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_UTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM CUTP
#define PROTECTION_NCM_CUTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_CUTP_DEFAULT_THRESHOLD (-100)                      // 0.1℃
#define PROTECTION_NCM_CUTP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_CUTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_CUTP_DEFAULT_RELEASE_THRESHOLD (-50)               // 0.1℃
#define PROTECTION_NCM_CUTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_CUTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM DUTP
#define PROTECTION_NCM_DUTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_DUTP_DEFAULT_THRESHOLD (-200)                      // 0.1℃
#define PROTECTION_NCM_DUTP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_DUTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_DUTP_DEFAULT_RELEASE_THRESHOLD (-50)               // 0.1℃
#define PROTECTION_NCM_DUTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_DUTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM RUTP
#define PROTECTION_NCM_RUTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_RUTP_DEFAULT_THRESHOLD (-200)                      // 0.1℃
#define PROTECTION_NCM_RUTP_DEFAULT_THRESHOLD_TICK (1000u * 3u)           // ms
#define PROTECTION_NCM_RUTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_RUTP_DEFAULT_RELEASE_THRESHOLD (-50)               // 0.1℃
#define PROTECTION_NCM_RUTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_RUTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM ODVP
#define PROTECTION_NCM_ODVP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_ODVP_DEFAULT_THRESHOLD (400)                       // mV
#define PROTECTION_NCM_ODVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_NCM_ODVP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_ODVP_DEFAULT_RELEASE_THRESHOLD (400)               // mV
#define PROTECTION_NCM_ODVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_ODVP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM CODVP
#define PROTECTION_NCM_CODVP_DEFAULT_ENABLED (1u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_CODVP_DEFAULT_THRESHOLD (400)               // mV
#define PROTECTION_NCM_CODVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_CODVP_DEFAULT_RELEASE_ENABLED (0u)          // 0: disabled, 1: enabled
#define PROTECTION_NCM_CODVP_DEFAULT_RELEASE_THRESHOLD (400)       // mV
#define PROTECTION_NCM_CODVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_CODVP_DEFAULT_PF_ENABLED (0u)  // 0: disabled, 1: enabled

// NCM DODVP
#define PROTECTION_NCM_DODVP_DEFAULT_ENABLED (1u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_DODVP_DEFAULT_THRESHOLD (400)               // mV
#define PROTECTION_NCM_DODVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_DODVP_DEFAULT_RELEASE_ENABLED (0u)          // 0: disabled, 1: enabled
#define PROTECTION_NCM_DODVP_DEFAULT_RELEASE_THRESHOLD (400)       // mV
#define PROTECTION_NCM_DODVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_DODVP_DEFAULT_PF_ENABLED (0u)  // 0: disabled, 1: enabled

// NCM RODVP
#define PROTECTION_NCM_RODVP_DEFAULT_ENABLED (1u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_RODVP_DEFAULT_THRESHOLD (400)               // mV
#define PROTECTION_NCM_RODVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_RODVP_DEFAULT_RELEASE_ENABLED (0u)          // 0: disabled, 1: enabled
#define PROTECTION_NCM_RODVP_DEFAULT_RELEASE_THRESHOLD (400)       // mV
#define PROTECTION_NCM_RODVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_RODVP_DEFAULT_PF_ENABLED (0u)  // 0: disabled, 1: enabled

// NCM ODTP
#define PROTECTION_NCM_ODTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_ODTP_DEFAULT_THRESHOLD (150)                       // 0.1℃
#define PROTECTION_NCM_ODTP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_NCM_ODTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_ODTP_DEFAULT_RELEASE_THRESHOLD (150)               // 0.1℃
#define PROTECTION_NCM_ODTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_ODTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM OPP
#define PROTECTION_NCM_OPP_DEFAULT_ENABLED (0u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_OPP_DEFAULT_THRESHOLD (200)                       // 0.1PSI
#define PROTECTION_NCM_OPP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_NCM_OPP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_OPP_DEFAULT_RELEASE_THRESHOLD (180)               // 0.1PSI
#define PROTECTION_NCM_OPP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_OPP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// NCM UPP
#define PROTECTION_NCM_UPP_DEFAULT_ENABLED (0u)                          // 0: disabled, 1: enabled
#define PROTECTION_NCM_UPP_DEFAULT_THRESHOLD (100)                       // 0.1PSI
#define PROTECTION_NCM_UPP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_NCM_UPP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_NCM_UPP_DEFAULT_RELEASE_THRESHOLD (140)               // 0.1PSI
#define PROTECTION_NCM_UPP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_UPP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

#define PROTECTION_NCM_ODVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_NCM_CODVP_CHECK_THRESHOLD (3600)               // mV

// LFP OVP
#define PROTECTION_LFP_OVP_DEFAULT_ENABLED (0u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_OVP_DEFAULT_THRESHOLD (3850)                      // mV
#define PROTECTION_LFP_OVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_OVP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_OVP_DEFAULT_RELEASE_THRESHOLD (3800)              // mV
#define PROTECTION_LFP_OVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_OVP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP UVP
#define PROTECTION_LFP_UVP_DEFAULT_ENABLED (0u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_UVP_DEFAULT_THRESHOLD (2400)                      // mV
#define PROTECTION_LFP_UVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_UVP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_UVP_DEFAULT_RELEASE_THRESHOLD (2500)              // mV
#define PROTECTION_LFP_UVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_UVP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP COCP
#define PROTECTION_LFP_COCP_DEFAULT_ENABLED (0u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_COCP_DEFAULT_THRESHOLD (600)                       // 0.1A
#define PROTECTION_LFP_COCP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_COCP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_COCP_DEFAULT_RELEASE_THRESHOLD (30)                // 0.1A
#define PROTECTION_LFP_COCP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_COCP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP DOCP
#define PROTECTION_LFP_DOCP_DEFAULT_ENABLED (0u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_DOCP_DEFAULT_THRESHOLD (-3500)                     // 0.1A
#define PROTECTION_LFP_DOCP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_DOCP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_DOCP_DEFAULT_RELEASE_THRESHOLD (-30)               // 0.1A
#define PROTECTION_LFP_DOCP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_DOCP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP ROCP
#define PROTECTION_LFP_ROCP_DEFAULT_ENABLED (0u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_ROCP_DEFAULT_THRESHOLD (3500)                      // 0.1A
#define PROTECTION_LFP_ROCP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_ROCP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_ROCP_DEFAULT_RELEASE_THRESHOLD (30)                // 0.1A
#define PROTECTION_LFP_ROCP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_ROCP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP OTP
#define PROTECTION_LFP_OTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_OTP_DEFAULT_THRESHOLD (600)                       // 0.1℃
#define PROTECTION_LFP_OTP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_OTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_OTP_DEFAULT_RELEASE_THRESHOLD (500)               // 0.1℃
#define PROTECTION_LFP_OTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_OTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP COTP
#define PROTECTION_LFP_COTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_COTP_DEFAULT_THRESHOLD (550)                       // 0.1℃
#define PROTECTION_LFP_COTP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_COTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_COTP_DEFAULT_RELEASE_THRESHOLD (500)               // 0.1℃
#define PROTECTION_LFP_COTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_COTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP DOTP
#define PROTECTION_LFP_DOTP_DEFAULT_ENABLED (1u)
#define PROTECTION_LFP_DOTP_DEFAULT_THRESHOLD (600)                       // 0.1℃
#define PROTECTION_LFP_DOTP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_DOTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_DOTP_DEFAULT_RELEASE_THRESHOLD (600)               // 0.1℃
#define PROTECTION_LFP_DOTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_DOTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP ROTP
#define PROTECTION_LFP_ROTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_ROTP_DEFAULT_THRESHOLD (600)                       // 0.1℃
#define PROTECTION_LFP_ROTP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_ROTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_ROTP_DEFAULT_RELEASE_THRESHOLD (500)               // 0.1℃
#define PROTECTION_LFP_ROTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_ROTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP UTP
#define PROTECTION_LFP_UTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_UTP_DEFAULT_THRESHOLD (-200)                      // 0.1℃
#define PROTECTION_LFP_UTP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_UTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_UTP_DEFAULT_RELEASE_THRESHOLD (-50)               // 0.1℃
#define PROTECTION_LFP_UTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_UTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP CUTP
#define PROTECTION_LFP_CUTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_CUTP_DEFAULT_THRESHOLD (-100)                      // 0.1℃
#define PROTECTION_LFP_CUTP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_CUTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_CUTP_DEFAULT_RELEASE_THRESHOLD (-50)               // 0.1℃
#define PROTECTION_LFP_CUTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_CUTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP DUTP
#define PROTECTION_LFP_DUTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_DUTP_DEFAULT_THRESHOLD (-200)                      // 0.1℃
#define PROTECTION_LFP_DUTP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_DUTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_DUTP_DEFAULT_RELEASE_THRESHOLD (-50)               // 0.1℃
#define PROTECTION_LFP_DUTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_DUTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP RUTP
#define PROTECTION_LFP_RUTP_DEFAULT_ENABLED (1u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_RUTP_DEFAULT_THRESHOLD (-200)                      // 0.1℃
#define PROTECTION_LFP_RUTP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_RUTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_RUTP_DEFAULT_RELEASE_THRESHOLD (-50)               // 0.1℃
#define PROTECTION_LFP_RUTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_RUTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP ODVP
#define PROTECTION_LFP_ODVP_DEFAULT_ENABLED (0u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_ODVP_DEFAULT_THRESHOLD (400)                       // mV
#define PROTECTION_LFP_ODVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_ODVP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_ODVP_DEFAULT_RELEASE_THRESHOLD (350)               // mV
#define PROTECTION_LFP_ODVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_ODVP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP CODVP
#define PROTECTION_LFP_CODVP_DEFAULT_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_CODVP_DEFAULT_THRESHOLD (400)               // mV
#define PROTECTION_LFP_CODVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_CODVP_DEFAULT_RELEASE_ENABLED (0u)          // 0: disabled, 1: enabled
#define PROTECTION_LFP_CODVP_DEFAULT_RELEASE_THRESHOLD (350)       // mV
#define PROTECTION_LFP_CODVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_CODVP_DEFAULT_PF_ENABLED (0u)  // 0: disabled, 1: enabled

// LFP DODVP
#define PROTECTION_LFP_DODVP_DEFAULT_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_DODVP_DEFAULT_THRESHOLD (400)               // mV
#define PROTECTION_LFP_DODVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_DODVP_DEFAULT_RELEASE_ENABLED (0u)          // 0: disabled, 1: enabled
#define PROTECTION_LFP_DODVP_DEFAULT_RELEASE_THRESHOLD (350)       // mV
#define PROTECTION_LFP_DODVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_DODVP_DEFAULT_PF_ENABLED (0u)  // 0: disabled, 1: enabled

// LFP RODVP
#define PROTECTION_LFP_RODVP_DEFAULT_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_RODVP_DEFAULT_THRESHOLD (400)               // mV
#define PROTECTION_LFP_RODVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_RODVP_DEFAULT_RELEASE_ENABLED (0u)          // 0: disabled, 1: enabled
#define PROTECTION_LFP_RODVP_DEFAULT_RELEASE_THRESHOLD (350)       // mV
#define PROTECTION_LFP_RODVP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_RODVP_DEFAULT_PF_ENABLED (0u)  // 0: disabled, 1: enabled

// LFP ODTP
#define PROTECTION_LFP_ODTP_DEFAULT_ENABLED (0u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_ODTP_DEFAULT_THRESHOLD (150)                       // 0.1℃
#define PROTECTION_LFP_ODTP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_ODTP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_ODTP_DEFAULT_RELEASE_THRESHOLD (100)               // 0.1℃
#define PROTECTION_LFP_ODTP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_ODTP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP OPP
#define PROTECTION_LFP_OPP_DEFAULT_ENABLED (0u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_OPP_DEFAULT_THRESHOLD (200)                       // 0.1PSI
#define PROTECTION_LFP_OPP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_OPP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_OPP_DEFAULT_RELEASE_THRESHOLD (180)               // 0.1PSI
#define PROTECTION_LFP_OPP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_OPP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

// LFP UPP
#define PROTECTION_LFP_UPP_DEFAULT_ENABLED (0u)                          // 0: disabled, 1: enabled
#define PROTECTION_LFP_UPP_DEFAULT_THRESHOLD (100)                       // 0.1PSI
#define PROTECTION_LFP_UPP_DEFAULT_THRESHOLD_TICK (1000u * 10u)          // ms
#define PROTECTION_LFP_UPP_DEFAULT_RELEASE_ENABLED (0u)                  // 0: disabled, 1: enabled
#define PROTECTION_LFP_UPP_DEFAULT_RELEASE_THRESHOLD (140)               // 0.1PSI
#define PROTECTION_LFP_UPP_DEFAULT_RELEASE_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_UPP_DEFAULT_PF_ENABLED (0u)                       // 0: disabled, 1: enabled

#define PROTECTION_LFP_ODVP_DEFAULT_THRESHOLD_TICK (1000u * 10u)  // ms
#define PROTECTION_LFP_CODVP_CHECK_THRESHOLD (3300)               // mV
/*==========================================================================*/

// TODO: remove legacy code below ===========================================

// Balancing legacy
/*==========================================================================*/
#define BALANCING_MIN_START_THRESHOLD_MIN_BANK_VOLTAGE (2700u)  // mV
#define BALANCING_MAX_START_THRESHOLD_MIN_BANK_VOLTAGE (5000u)  // mV
#define BALANCING_MIN_START_THRESHOLD_DV (5u)                   // mV
#define BALANCING_MAX_START_THRESHOLD_DV (400u)                 // mV
#define BALANCING_MIN_START_THRESHOLD_COUNT (1u)
#define BALANCING_MAX_START_THRESHOLD_COUNT (1000u)
#define BALANCING_MIN_GOING_TICK (1000u)                                     // ms
#define BALANCING_MAX_GOING_TICK (10000u)                                    // ms
#define BALANCING_MIN_RELAXATION_TICK (BANK_VOLTAGE_READ_INTERVAL_MIN_TICK)  // ms
#define BALANCING_MAX_RELAXATION_TICK (10000u)                               // ms
#define BALANCING_MIN_ODD_EVEN_SWITCHING_TICK (500u)                         // ms
#define BALANCING_TEMPERATURE_COUNT (1u)
/*==========================================================================*/

#endif
