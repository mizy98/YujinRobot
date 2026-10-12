#ifndef BMS_TYPE_H
#define BMS_TYPE_H

#include <stdint.h>

enum _BmsProtectionCode {
    BMS_PROTECTION_CODE_OVP,
    BMS_PROTECTION_CODE_UVP,

    BMS_PROTECTION_CODE_COCP,
    BMS_PROTECTION_CODE_DOCP,
    BMS_PROTECTION_CODE_ROCP,

    BMS_PROTECTION_CODE_OTP,
    BMS_PROTECTION_CODE_COTP,
    BMS_PROTECTION_CODE_DOTP,
    BMS_PROTECTION_CODE_ROTP,
    BMS_PROTECTION_CODE_UTP,
    BMS_PROTECTION_CODE_CUTP,
    BMS_PROTECTION_CODE_DUTP,
    BMS_PROTECTION_CODE_RUTP,

    BMS_PROTECTION_CODE_ODVP,
    BMS_PROTECTION_CODE_CODVP,
    BMS_PROTECTION_CODE_DODVP,
    BMS_PROTECTION_CODE_RODVP,

    BMS_PROTECTION_CODE_ODTP,

    BMS_PROTECTION_CODE_OPP,
    BMS_PROTECTION_CODE_UPP,

    BMS_PROTECTION_CODE_MAX
};
typedef uint8_t BmsProtectionCode;

enum _BmsWarningCode {
    BMS_WARNING_CODE_OVW,
    BMS_WARNING_CODE_UVW,

    BMS_WARNING_CODE_OTW,
    BMS_WARNING_CODE_UTW,

    BMS_WARNING_CODE_MAX
};
typedef uint8_t BmsWarningCode;

enum _BmsPackState {
    BMS_PACK_STATE_IDLE,
    BMS_PACK_STATE_CHARGE,
    BMS_PACK_STATE_DISCHARGE,
    BMS_PACK_STATE_REGENERATE,
    BMS_PACK_STATE_SLEEP,
    BMS_PACK_STATE_PROTECTION,
    BMS_PACK_STATE_PERMANENT_FAILURE,
    BMS_PACK_STATE_CHARGE_CC,
    BMS_PACK_STATE_CHARGE_CV,
};
typedef uint8_t BmsPackState;

enum _BmsDeviceState {
    BMS_DEVICE_STATE_OFF,
    BMS_DEVICE_STATE_ON,
    BMS_DEVICE_STATE_ERROR,
};
typedef uint8_t BmsDeviceState;

enum _BmsContactorState {
    BMS_CONTACTOR_STATE_OFF,
    BMS_CONTACTOR_STATE_ON,
    BMS_CONTACTOR_STATE_PRECHARGE,
    BMS_CONTACTOR_STATE_ERROR,
};
typedef uint8_t BmsContactorState;

enum _BmsChargerMode {
    BMS_CHARGER_MODE_DISABLE,
    BMS_CHARGER_MODE_INTERNAL_VOLTAGE,
    BMS_CHARGER_MODE_EXTERNAL_VOLTAGE,
};
typedef uint8_t BmsChargerMode;

enum _BmsSleepMode {
    BMS_SLEEP_MODE_DISABLE,
    BMS_SLEEP_MODE_SOC,
    BMS_SLEEP_MODE_VOLTAGE,
};
typedef uint8_t BmsSleepMode;

enum _BmsChargeParseMode {
    // CC mode (enter charge state when the CC(constant current) is detected)
    BMS_CHARGE_PARSE_MODE_CC,

    // charge signal mode (enter charge state when the charge signal pin is low)
    BMS_CHARGE_PARSE_MODE_SIGNAL,

    // time mode (enter charge state when the current is higher than the charge threshold current
    // in N [ms])
    BMS_CHARGE_PARSE_MODE_TIME,
};
typedef uint8_t BmsChargeParseMode;

enum _BmsBdiType {
    BMS_BDI_TYPE_DISABLE = 0,
    BMS_BDI_TYPE_MZN58_CAN = (1 << 0),
    BMS_BDI_TYPE_LP5018_I2C = (1 << 1),
};
typedef uint8_t BmsBdiType;

enum _BmsContactorMode {
    BMS_CONTACTOR_MODE_MAIN,
    BMS_CONTACTOR_MODE_MAIN_PRECHARGE,
    BMS_CONTACTOR_MODE_CHARGE_DISCHARGE,
    BMS_CONTACTOR_MODE_CHARGE_DISCHARGE_PRECHARGE,
};
typedef uint8_t BmsContactorMode;

#endif