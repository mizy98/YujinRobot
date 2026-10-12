#include "contactor.h"

#include "../interface/rtos.h"
#include "../interface/signal.h"
#include "../interface/system_time.h"

#define self (contactor_private)

#define CONTACTOR_ERROR_THRESHOLD_TICK (6000u)

typedef enum {
    CONTACTOR_PHASE_IDLE,         // 현재 상태 유지 중
    CONTACTOR_PHASE_PRECHARGING,  // precharge 릴레이 ON, 메인 릴레이 대기 중
    CONTACTOR_PHASE_CHECKING,     // 메인 릴레이 ON 후 상태 검증 대기 중
} ContactorPhase;

typedef struct {
    uint32_t check_threshold_tick;
    uint32_t precharge_tick;
    ContactorType type;
} ContactorConfig;

typedef struct {
    BmsContactorState state;
    BmsContactorState expected_state;
    BmsContactorState pending_state;

    ContactorPhase phase;
    uint32_t phase_start_tick;
    uint32_t mismatch_start_tick;

    ContactorConfig config;

    void (*setPrecharge)(bool state);
    bool (*isPrechargeOn)(void);

    void (*setMain)(bool state);
    bool (*isMainOn)(void);
} Contactor;

static struct {
    AppErrorInfo error_info;

    Contactor main;
    Contactor charge;
    Contactor discharge;

    ContactorCallback callback;
    BmsContactorConfig config;
} contactor_private;

static bool _isValidState(BmsContactorState state) {
    return (state == BMS_CONTACTOR_STATE_OFF || state == BMS_CONTACTOR_STATE_ON);
}

static uint32_t _elapsedTick(uint32_t start_tick) { return SystemTime_GetTick() - start_tick; }

static bool _isPrechargeUsed(Contactor* c) {
    return c->config.precharge_tick > 0 && c->setPrecharge != NULL;
}

static void _processContactor(Contactor* c) {
    uint32_t now = SystemTime_GetTick();

    switch (c->phase) {
        case CONTACTOR_PHASE_PRECHARGING: {
            uint32_t precharge_tick = (c->config.type == CONTACTOR_TYPE_MAIN)
                                          ? self.config.main_contactor_precharge_tick
                                      : (c->config.type == CONTACTOR_TYPE_CHARGE)
                                          ? self.config.charge_contactor_precharge_tick
                                          : self.config.discharge_contactor_precharge_tick;

            if (_elapsedTick(c->phase_start_tick) < precharge_tick) {
                break;
            }

            c->setMain(true);

            if (_isPrechargeUsed(c)) {
                Rtos_SleepMs(10);
                c->setPrecharge(false);
            }

            c->expected_state = c->pending_state;

            c->phase = CONTACTOR_PHASE_CHECKING;
            c->phase_start_tick = now;
            break;
        }

        case CONTACTOR_PHASE_CHECKING: {
            if (c->config.check_threshold_tick == 0) {
                BmsContactorState prev = c->state;
                c->state = c->expected_state;

                if (prev != c->state && self.callback.onStateChanged != NULL) {
                    self.callback.onStateChanged(c->config.type, c->state);
                }

                c->phase = CONTACTOR_PHASE_IDLE;
                break;
            }

            if (_elapsedTick(c->phase_start_tick) < c->config.check_threshold_tick) {
                break;
            }

            bool signal_on = c->isMainOn();
            bool expected_on = (c->expected_state == BMS_CONTACTOR_STATE_ON);

            if (signal_on != expected_on) {
                if (c->mismatch_start_tick == 0) {
                    c->mismatch_start_tick = SystemTime_GetTick();
                    break;
                }

                if (_elapsedTick(c->mismatch_start_tick) < CONTACTOR_ERROR_THRESHOLD_TICK) {
                    break;
                }

                c->mismatch_start_tick = 0;

                if (self.callback.onError != NULL) {
                    self.callback.onError(
                        c->config.type, 0, c->expected_state,
                        signal_on ? BMS_CONTACTOR_STATE_ON : BMS_CONTACTOR_STATE_OFF);
                }

                c->setMain(false);
                if (_isPrechargeUsed(c)) {
                    c->setPrecharge(false);
                }
                c->state = BMS_CONTACTOR_STATE_OFF;
                c->expected_state = BMS_CONTACTOR_STATE_OFF;
                c->pending_state = BMS_CONTACTOR_STATE_OFF;
                c->phase = CONTACTOR_PHASE_IDLE;

            } else {
                c->mismatch_start_tick = 0;

                BmsContactorState prev = c->state;
                c->state = c->expected_state;

                if (prev != c->state && self.callback.onStateChanged != NULL) {
                    self.callback.onStateChanged(c->config.type, c->state);
                }

                c->phase = CONTACTOR_PHASE_IDLE;
            }
            break;
        }

        case CONTACTOR_PHASE_IDLE:
        default:
            break;
    }
}

AppResult Contactor_Initialize(BmsConfig* config, ContactorCallback* callback) {
    if (config == NULL || callback == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CONTACTOR_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    self.main.config.type = CONTACTOR_TYPE_MAIN;
    self.main.config.precharge_tick = config->contactor.main_contactor_precharge_tick;
    self.main.config.check_threshold_tick = config->contactor.main_contactor_check_threshold_tick;
    self.main.setPrecharge = Signal_SetMainPrechargeContactor;
    self.main.isPrechargeOn = Signal_IsMainPrechargeContactorOn;
    self.main.setMain = Signal_SetMainContactor;
    self.main.isMainOn = Signal_IsMainContactorOn;

    self.charge.config.type = CONTACTOR_TYPE_CHARGE;
    self.charge.config.precharge_tick = config->contactor.charge_contactor_precharge_tick;
    self.charge.config.check_threshold_tick =
        config->contactor.charge_contactor_check_threshold_tick;
    self.charge.setPrecharge = Signal_SetChargePrechargeContactor;
    self.charge.isPrechargeOn = Signal_IsChargePrechargeContactorOn;
    self.charge.setMain = Signal_SetChargeContactor;
    self.charge.isMainOn = Signal_IsChargeContactorOn;

    self.discharge.config.type = CONTACTOR_TYPE_DISCHARGE;
    self.discharge.config.precharge_tick = config->contactor.discharge_contactor_precharge_tick;
    self.discharge.config.check_threshold_tick =
        config->contactor.discharge_contactor_check_threshold_tick;
    self.discharge.setPrecharge = Signal_SetDischargePrechargeContactor;
    self.discharge.isPrechargeOn = Signal_IsDischargePrechargeContactorOn;
    self.discharge.setMain = Signal_SetDischargeContactor;
    self.discharge.isMainOn = Signal_IsDischargeContactorOn;

    self.main.state = BMS_CONTACTOR_STATE_OFF;
    self.charge.state = BMS_CONTACTOR_STATE_OFF;
    self.discharge.state = BMS_CONTACTOR_STATE_OFF;

    self.main.phase = CONTACTOR_PHASE_IDLE;
    self.charge.phase = CONTACTOR_PHASE_IDLE;
    self.discharge.phase = CONTACTOR_PHASE_IDLE;

    self.config = config->contactor;
    self.callback = *callback;

    return APP_RESULT_OK;
}

AppResult Contactor_Deinitialize(void) { return APP_RESULT_OK; }

void Contactor_GetLastErrorInfo(AppErrorInfo* error_info) {
    if (error_info == NULL) return;
    *error_info = self.error_info;
}

void Contactor_SetState(ContactorType type, BmsContactorState state) {
    if (!_isValidState(state)) return;

    Contactor* contactor = NULL;
    switch (type) {
        case CONTACTOR_TYPE_MAIN:
            contactor = &self.main;
            break;
        case CONTACTOR_TYPE_CHARGE:
            contactor = &self.charge;
            break;
        case CONTACTOR_TYPE_DISCHARGE:
            contactor = &self.discharge;
            break;
        default:
            return;
    }

    if (contactor->phase != CONTACTOR_PHASE_IDLE) {
        return;
    }

    if (contactor->state == state) {
        return;
    }

    if (state == BMS_CONTACTOR_STATE_OFF) {
        if (_isPrechargeUsed(contactor)) {
            contactor->setPrecharge(false);
        }
        contactor->setMain(false);
        contactor->expected_state = BMS_CONTACTOR_STATE_OFF;
        contactor->pending_state = BMS_CONTACTOR_STATE_OFF;

        contactor->phase = CONTACTOR_PHASE_CHECKING;
        contactor->phase_start_tick = SystemTime_GetTick();

    } else {
        bool needs_precharge =
            _isPrechargeUsed(contactor) && (contactor->state == BMS_CONTACTOR_STATE_OFF);

        if (needs_precharge) {
            contactor->setPrecharge(true);
            contactor->pending_state = BMS_CONTACTOR_STATE_ON;
            contactor->phase = CONTACTOR_PHASE_PRECHARGING;
            contactor->phase_start_tick = SystemTime_GetTick();

            contactor->state = BMS_CONTACTOR_STATE_PRECHARGE;
            if (self.callback.onStateChanged != NULL) {
                self.callback.onStateChanged(contactor->config.type, BMS_CONTACTOR_STATE_PRECHARGE);
            }
        } else {
            contactor->setMain(true);
            contactor->expected_state = BMS_CONTACTOR_STATE_ON;
            contactor->phase = CONTACTOR_PHASE_CHECKING;
            contactor->phase_start_tick = SystemTime_GetTick();
        }
    }
}

void Contactor_GetState(ContactorType type, BmsContactorState* state) {
    if (state == NULL) return;

    switch (type) {
        case CONTACTOR_TYPE_MAIN:
            *state = self.main.state;
            break;
        case CONTACTOR_TYPE_CHARGE:
            *state = self.charge.state;
            break;
        case CONTACTOR_TYPE_DISCHARGE:
            *state = self.discharge.state;
            break;
        default:
            break;
    }
}

void Contactor_Process(void) {
    _processContactor(&self.main);
    _processContactor(&self.charge);
    _processContactor(&self.discharge);
}