#ifndef CHARGER_CAN_H
#define CHARGER_CAN_H

#include "../model/app_model.h"

typedef enum {
    CHARGER_CAN_TYPE_DEFAULT,
} ChargerCanType;

typedef enum {
    CHARGER_CAN_MODE_INTERNAL_VOLTAGE,
    CHARGER_CAN_MODE_CHARGER_VOLTAGE,
} ChargerCanMode;

typedef enum {
    CHARGER_CAN_STATE_IDLE,
    CHARGER_CAN_STATE_CHARGE,
    CHARGER_CAN_STATE_RECHARGE,
} ChargerCanState;

#pragma pack(push, 1)

typedef struct {
    uint16_t voltage;  // 0.1V
    uint16_t current;  // 0.1A

    struct {
        uint8_t is_hardware_failure : 1;
        uint8_t is_temperature_over : 1;
        uint8_t is_input_voltage_wrong : 1;
        uint8_t is_connected : 1;
        uint8_t is_communication_timeout : 1;
        uint8_t reserved : 3;
    } error;

} ChargerCanStatus;

#pragma pack(pop)

typedef struct {
    ChargerCanType type;
    ChargerCanMode mode;

    uint32_t check_tick;
    uint32_t timeout_tick;

    uint16_t cc_current;         // 0.1A
    uint16_t cv_voltage;         // 0.1V
    uint16_t cv_cutoff_current;  // 0.1A

    uint16_t recharge_start_voltage;  // 0.1V
} ChargerCanConfig;

typedef struct {
    void (*onConnected)();
    void (*onDisconnected)();
    void (*onChargeStarted)();
    void (*onChargeFinished)();
} ChargerCanCallback;

AppResult ChargerCan_Deinitialize(void);
AppResult ChargerCan_Initialize(const BmsConfig* config, const ChargerCanCallback* callback);
void ChargerCan_GetLastErrorInfo(AppErrorInfo* info);
AppResult ChargerCan_Process(const BmsData* data);
ChargerCanState ChargerCan_GetState();
void ChargerCan_GetStatus(ChargerCanStatus* status);

#endif