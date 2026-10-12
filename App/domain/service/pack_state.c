#include "pack_state.h"

#include "../interface/signal.h"
#include "../interface/system_time.h"

#define self (pack_state_private)

typedef struct {
    bool is_parsing;
    uint32_t parse_start_tick;  // ms
    uint32_t elapsed_tick;      // ms
} TimeChargeParser;

typedef struct {
    bool is_parsing;
    uint32_t last_parse_tick;   // ms
    uint32_t parse_start_tick;  // ms

    int16_t max_current;        // 0.1A
    int16_t min_current;        // 0.1A
    int16_t current_variation;  // 0.1A

    float filtered_current;  // A
} CCChargeParser;

typedef struct {
    uint8_t temp;
} CVChargeParser;

static struct {
    AppErrorInfo error_info;

    BmsPackState state;

    uint32_t charge_last_tick;
    uint32_t last_parse_tick;  // ms

    int16_t last_current;

    TimeChargeParser time_charge;
    CCChargeParser cc_charge;
    CVChargeParser cv_charge;

    BmsStateConfig state_config;
    BmsSleepConfig sleep_config;

    // Sleep state tracking
    bool sleep_condition_active;
    uint32_t sleep_condition_start_tick;
    bool wakeup_condition_active;
    uint32_t wakeup_condition_start_tick;

    void (*callback)(BmsPackState state);
} pack_state_private;

static BmsPackState _parseProtectionState(BmsProtectionState protection, BmsProtectionState pf) {
    if (protection.value > 0) {
        return BMS_PACK_STATE_PROTECTION;

    } else if (pf.value > 0) {
        return BMS_PACK_STATE_PERMANENT_FAILURE;

    } else {
        return BMS_PACK_STATE_IDLE;
    }
}

static bool _isConstantCurrentCharge(CCChargeParser* parser, int16_t current,
                                     uint32_t current_tick) {
    if (current < self.state_config.charge_threshold_current) {
        parser->is_parsing = false;
        parser->parse_start_tick = 0;
        return false;
    }

    bool is_elapsed =
        (current_tick - parser->last_parse_tick) >= self.state_config.cc_parse_sampling_tick;
    if (is_elapsed) {
        if (parser->is_parsing) {
            if (current > parser->max_current) parser->max_current = current;
            if (current < parser->min_current) parser->min_current = current;

            parser->current_variation = parser->max_current - parser->min_current;
            if (parser->current_variation > self.state_config.cc_parse_stable_threshold_current) {
                parser->parse_start_tick = current_tick;
                parser->max_current = current;
                parser->min_current = current;
            }
        } else {
            parser->is_parsing = true;
            parser->parse_start_tick = current_tick;
            parser->max_current = current;
            parser->min_current = current;
        }

        parser->last_parse_tick = current_tick;
    }

    if (parser->is_parsing) {
        uint32_t stable_duration = current_tick - parser->parse_start_tick;

        return stable_duration >= self.state_config.cc_parse_threshold_tick;
    }

    return false;
}

// TODO: CV parser
// static bool _isConstantVoltageCharge(CVChargeParser* parser, int16_t current,
//                                      uint32_t current_tick) {
//     static int16_t last_current = 0;
//     static uint32_t last_update_tick = 0;
//     static uint32_t monitoring_start_tick = 0;
//     static int16_t max_current_in_window = 0;
//     static int16_t initial_current = 0;
//     static uint8_t decreasing_trend_count = 0;
//     static bool monitoring_active = false;

//     const uint32_t SAMPLING_INTERVAL = 500;        // 500ms 간격으로 샘플링 (더 긴 간격)
//     const uint32_t MONITORING_WINDOW = 5000;       // 5초간 모니터링
//     const int16_t CV_MIN_CURRENT = 5;              // 0.5A 이상 (0.1A 단위)
//     const int16_t CURRENT_DECREASE_THRESHOLD = 5;  // 0.5A 감소 임계값
//     const uint8_t MIN_DECREASING_SAMPLES = 3;      // 최소 3번의 감소 패턴
//     const int16_t CURRENT_INCREASE_TOLERANCE = 3;  // 0.3A 증가 허용치

//     // CV 모드 감지를 위한 최소 충전 전류 확인
//     if (current < CV_MIN_CURRENT) {
//         monitoring_active = false;
//         monitoring_start_tick = 0;
//         decreasing_trend_count = 0;
//         return false;
//     }

//     // 정해진 간격마다 전류 체크
//     if (current_tick - last_update_tick >= SAMPLING_INTERVAL) {
//         // 모니터링 시작
//         if (!monitoring_active) {
//             monitoring_active = true;
//             monitoring_start_tick = current_tick;
//             initial_current = current;
//             max_current_in_window = current;
//             decreasing_trend_count = 0;
//         } else {
//             // 전류 감소 트렌드 감지
//             if (last_current > 0) {
//                 // 이전 전류보다 현재 전류가 감소했는지 확인
//                 if ((last_current - current) >= CURRENT_DECREASE_THRESHOLD) {
//                     decreasing_trend_count++;
//                 } else if (current > last_current + CURRENT_INCREASE_TOLERANCE) {
//                     // 전류가 증가하면 CV가 아님 - 모니터링 재시작
//                     monitoring_start_tick = current_tick;
//                     initial_current = current;
//                     decreasing_trend_count = 0;
//                 }
//             }

//             // 최대 전류 업데이트 (참조용)
//             if (current > max_current_in_window) {
//                 max_current_in_window = current;
//             }
//         }

//         last_update_tick = current_tick;
//         last_current = current;
//     }

//     // CV 모드 판정 조건 확인
//     if (monitoring_active) {
//         uint32_t monitoring_duration = current_tick - monitoring_start_tick;

//         // 충분한 시간 동안 모니터링했고, 감소 트렌드가 확인된 경우
//         if (monitoring_duration >= MONITORING_WINDOW &&
//             decreasing_trend_count >= MIN_DECREASING_SAMPLES) {
//             // 초기 전류 대비 현재 전류가 충분히 감소했는지 확인
//             int16_t total_decrease = initial_current - current;
//             if (total_decrease >= (initial_current / 4)) {  // 25% 이상 감소
//                 return true;
//             }
//         }
//     }

//     return false;
// }

static BmsPackState _parseTimeCharge(TimeChargeParser* parser, int16_t current,
                                     uint32_t current_tick) {
    if (current >= self.state_config.charge_threshold_current) {
        if (!parser->is_parsing) {
            parser->is_parsing = true;
            parser->parse_start_tick = current_tick;
            return BMS_PACK_STATE_REGENERATE;
        }

        parser->elapsed_tick = current_tick - parser->parse_start_tick;
        if (parser->elapsed_tick >= self.state_config.charge_threshold_tick) {
            switch (self.state_config.charge_parse_mode) {
                case BMS_CHARGE_PARSE_MODE_TIME: {
                    return BMS_PACK_STATE_CHARGE;
                }

                case BMS_CHARGE_PARSE_MODE_SIGNAL: {
                    if (Signal_IsChargeOn()) {
                        return BMS_PACK_STATE_CHARGE;
                    } else {
                        return BMS_PACK_STATE_REGENERATE;
                    }
                }

                default: {
                    return BMS_PACK_STATE_CHARGE;
                }
            }
        } else {
            return BMS_PACK_STATE_REGENERATE;
        }

    } else if (current <= self.state_config.discharge_threshold_current) {
        parser->is_parsing = false;
        parser->parse_start_tick = 0;
        parser->elapsed_tick = 0;
        return BMS_PACK_STATE_DISCHARGE;

    } else {
        parser->is_parsing = false;
        parser->parse_start_tick = 0;
        parser->elapsed_tick = 0;
        return BMS_PACK_STATE_IDLE;
    }
}

static BmsPackState _parseChargeState(uint32_t current_tick, int16_t current) {
    if (self.state_config.charge_parse_mode == BMS_CHARGE_PARSE_MODE_CC) {
        if (_isConstantCurrentCharge(&self.cc_charge, current, current_tick)) {
            return BMS_PACK_STATE_CHARGE_CC;
        }
        // TODO: CV checker
        // else if (_isConstantVoltageCharge(&self.cv_charge, current, current_tick)) {
        //     return BMS_PACK_STATE_CHARGE_CV;
        // }

        else if (current >= self.state_config.charge_threshold_current) {
            return BMS_PACK_STATE_REGENERATE;
        } else if (current <= self.state_config.discharge_threshold_current) {
            return BMS_PACK_STATE_DISCHARGE;
        } else {
            return BMS_PACK_STATE_IDLE;
        }
    } else {
        return _parseTimeCharge(&self.time_charge, current, current_tick);
    }
}

static BmsPackState _parseSleepState(uint32_t current_tick, uint16_t soc,
                                     uint16_t min_cell_voltage) {
    if (self.sleep_config.mode == BMS_SLEEP_MODE_DISABLE) {
        self.sleep_condition_active = false;
        return BMS_PACK_STATE_IDLE;
    }

    if (self.state == BMS_PACK_STATE_SLEEP) {
        bool wake_up_condition_met = false;

        if (Signal_IsChargeOn() || Signal_IsWakeUpOn()) {
            self.wakeup_condition_active = true;
            self.wakeup_condition_start_tick = current_tick;
        }

        if (wake_up_condition_met) {
            if (!self.wakeup_condition_active) {
                self.wakeup_condition_active = true;
                self.wakeup_condition_start_tick = current_tick;
            } else {
                uint32_t wakeup_elapsed_tick = current_tick - self.wakeup_condition_start_tick;
                if (wakeup_elapsed_tick >= STATE_SLEEP_WAKEUP_SIGNAL_THRESHOLD_TICK) {
                    self.wakeup_condition_active = false;
                    self.sleep_condition_active = false;
                    return BMS_PACK_STATE_IDLE;
                }
            }
        } else {
            self.wakeup_condition_active = false;
            return BMS_PACK_STATE_SLEEP;
        }
    }

    if (self.state != BMS_PACK_STATE_IDLE) {
        self.sleep_condition_active = false;
        return BMS_PACK_STATE_IDLE;
    }

    bool sleep_condition_met = false;

    if (self.sleep_config.mode == BMS_SLEEP_MODE_SOC) {
        sleep_condition_met = (soc < self.sleep_config.threshold_soc);
    } else if (self.sleep_config.mode == BMS_SLEEP_MODE_VOLTAGE) {
        sleep_condition_met = (min_cell_voltage < self.sleep_config.threshold_voltage);
    }

    if (sleep_condition_met) {
        if (!self.sleep_condition_active) {
            self.sleep_condition_active = true;
            self.sleep_condition_start_tick = current_tick;
        } else {
            uint32_t elapsed_tick = current_tick - self.sleep_condition_start_tick;
            if (elapsed_tick >= self.sleep_config.threshold_tick) {
                return BMS_PACK_STATE_SLEEP;
            }
        }
    } else {
        self.sleep_condition_active = false;
    }

    return BMS_PACK_STATE_IDLE;
}

static BmsPackState _processStateChange(int16_t current, uint32_t current_tick,
                                        BmsPackState state) {
    bool is_state_changed = state != self.state;

    if (is_state_changed) {
        self.state = state;
        if (self.callback != NULL) {
            self.callback(self.state);
        }
    }

    self.last_parse_tick = current_tick;
    self.last_current = current;

    return self.state;
}

AppResult PackState_Initialize(BmsConfig* config, void (*callback)(BmsPackState state)) {
    if (config == NULL || callback == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_PACK_STATE_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    self.callback = callback;
    self.state = 0xFF;

    self.state_config = config->state;
    self.sleep_config = config->sleep;

    self.charge_last_tick = 0;
    self.last_parse_tick = 0;
    self.last_current = 0;

    self.time_charge.is_parsing = false;
    self.time_charge.parse_start_tick = 0;
    self.time_charge.elapsed_tick = 0;

    self.cc_charge.is_parsing = false;
    self.cc_charge.last_parse_tick = 0;
    self.cc_charge.parse_start_tick = 0;
    self.cc_charge.max_current = INT16_MIN;
    self.cc_charge.min_current = INT16_MAX;
    self.cc_charge.current_variation = 0;

    self.sleep_condition_active = false;
    self.sleep_condition_start_tick = 0;
    self.wakeup_condition_active = false;
    self.wakeup_condition_start_tick = 0;

    // TODO: reset CV parser state when implemented

    self.error_info = (AppErrorInfo){0};

    return APP_RESULT_OK;
}

void PackState_GetErrorInfo(AppErrorInfo* error_info) {
    if (error_info != NULL) {
        *error_info = self.error_info;
    }
}

BmsPackState PackState_ParseState(int16_t current, uint16_t pack_voltage,
                                  BmsProtectionState protection, BmsProtectionState pf,
                                  uint16_t soc, uint16_t min_bank_voltage) {
    uint32_t current_tick = SystemTime_GetTick();

    BmsPackState state = BMS_PACK_STATE_IDLE;

    state = _parseProtectionState(protection, pf);
    if (state != BMS_PACK_STATE_IDLE) {
        return _processStateChange(current, current_tick, state);
    }

    state = _parseChargeState(current_tick, current);
    if (state != BMS_PACK_STATE_IDLE) {
        return _processStateChange(current, current_tick, state);
    }

    state = _parseSleepState(current_tick, soc, min_bank_voltage);
    if (state != BMS_PACK_STATE_IDLE) {
        return _processStateChange(current, current_tick, state);
    }

    return _processStateChange(current, current_tick, state);
}

void PackState_WakeUp(void) {
    if (self.state == BMS_PACK_STATE_SLEEP) {
        self.state = BMS_PACK_STATE_IDLE;
        if (self.callback != NULL) {
            self.callback(self.state);
        }
    }
}

BmsPackState PackState_GetCurrentState(void) { return self.state; }

// TEST
#if 0
#include <math.h>

static float Filter_Ema(float previous_data, float new_data, float alpha) {
    return alpha * new_data + (1 - alpha) * previous_data;
}

static float filtered_value = 0;

static const float alpha = 0.003329;  // 0.1A 변화에 약 30초 동안 반응하는 EMA 필터의 알파 값

static const float charge_threshold = 3.0f;
static const float cc_charge_threshold = 2.5f;

static bool is_contant_current_charge(float data) {
    if (data < charge_threshold) {
        filtered_value = 0.0f;
        return false;
    }

    float delta = fabsf(filtered_value - data);
    if (delta < cc_charge_threshold) {
        return true;
    }

    filtered_value = Filter_Ema(filtered_value, data, alpha);

    return false;
}

static bool is_initialized = false;
static float filtered_value = 0.0f;

static const float alpha = 0.003329f;
static const float charge_threshold = 3.0f;
static const float cc_charge_threshold = 2.5f;

static bool is_constant_current_charge(float data) {
    // 충전 전류 미달 시 리셋
    if (data < charge_threshold) {
        is_initialized = false;
        filtered_value = 0.0f;
        return false;
    }

    // 첫 유효 샘플로 초기화 (0 수렴 문제 방지)
    if (!is_initialized) {
        filtered_value = data;
        is_initialized = true;
        return false;
    }

    // EMA는 매 호출마다 업데이트
    filtered_value = Filter_Ema(filtered_value, data, alpha);

    // 수렴 후 안정 여부 판단
    float delta = fabsf(filtered_value - data);
    return (delta < cc_charge_threshold);
}

#endif
