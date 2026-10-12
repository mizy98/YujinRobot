#include "soc.h"

#include <math.h>
#include <stdbool.h>

#include "../interface/system_time.h"
#include "soc_lfp.h"
#include "soc_ncm.h"

#define self (soc_private)

#define TRACK_DEADBAND (0.2f)         // Minimum SOC difference (%) to trigger follow behavior
#define TRACK_TICK_INTERVAL (10000u)  // Minimum interval (ms) between track adjustments
#define TRACK_MAX_C_RATE (3.0f)       // Maximum C-rate for correction factor
#define TRACK_HIGH_RATE (3.0f)
#define TRACK_LOW_RATE (1.0f)

#define REMAINING_TIME_CHECK_INTERVAL (1000u * 60u)  // ms

#define CYCLE_CHARGE_CURRENT_THRESHOLD (1.0f)  // A

#define LFP_FULL_CHARGE_REGION_THRESHOLD_TICK (1000u * 30u)  // ms

typedef struct {
    float soc;            // %
    float coulomb;        // coulomb
    float full_capacity;  // Ah
} SocData;

static struct {
    SocData real_data;
    SocData report_data;
    SocData coulomb_data;

    float nominal_capacity;  // Ah

    float real_report_diff_soc;  // %

    uint32_t last_calculate_tick;  // ms

    uint32_t track_tick_count;  // ms
    uint32_t last_track_tick;   // ms

    uint32_t remaining_check_tick;      // ms
    uint32_t charge_remaining_time;     // ms
    uint32_t discharge_remaining_time;  // ms
    BmsPackState last_pack_state;

    struct {
        float cycle;             // cycle
        float charged_coulomb;   // As
        float charged_capacity;  // Ah
        float charged_c_rate;
        float charged_time;  // s
        bool was_charging;
    } cycle;

    struct {
        uint32_t capacity_calibration_tick_count;  // ms
        uint32_t last_capacity_calibration_tick;   // ms

        uint32_t ocv_calibration_tick_count;  // ms
        uint32_t last_ocv_calibration_tick;   // ms

        uint32_t last_full_charge_clmap_tick;   // ms
        uint32_t full_charge_clmap_tick_count;  // ms
    } calibration;

    // float track_kp;     // proportional gain for SOC tracking
    // float track_ki;     // integral gain for SOC tracking (if using integral control)
    // float track_delta;  // adaptive delta for SOC correction based on error magnitude and C-rate

    struct {
        BmsCellModel cell_model;
        uint32_t ocv_calibration_tick;       // ms
        uint32_t capacity_calibration_tick;  // ms
        float min_track_unit;                // %
        uint32_t track_tick;                 // ms
    } config;

    AppErrorInfo error_info;
} soc_private;

static bool _isFullCharge(BmsCellModel cell_model, uint16_t voltage, int16_t temperature) {
    switch (cell_model) {
        case BMS_CELL_MODEL_EVE_LF105:
        case BMS_CELL_MODEL_EVE_LF150:
            return SocLfp_IsFullChargeRegion(temperature, voltage);
            break;

        default:
            return false;
    }
}

/*
static void _calibrate(BmsCellModel cell_model, SocData* data, uint16_t ocv, int16_t temperature,
                       bool is_force) {
    float capacity = 0.0f;
    float soc = 0.0f;

    switch (cell_model) {
        case BMS_CELL_MODEL_LGES_INR21700_M50LT:
        case BMS_CELL_MODEL_SKON_E600:
            soc = SocNcm_CalculateSoc(temperature, ocv);
            capacity = SocNcm_CalculateCapacityRatioByTemperature(temperature) *
self.nominal_capacity; break;

        case BMS_CELL_MODEL_EVE_LF105:
        case BMS_CELL_MODEL_EVE_LF150:
            if (is_force || SocLfp_IsCalibrationRegion(temperature, ocv)) {
                soc = SocLfp_CalculateSoc(temperature, ocv);
            }
            capacity = SocLfp_CalculateCapacityRatioByTemperature(temperature) *
self.nominal_capacity; break;

        default:
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_SOC_UNSUPPORTED_CELL_MODEL);
            return;
    }

    data->soc = soc;
    data->full_capacity = capacity;
    data->coulomb = (soc / 100.0f) * capacity * 3600.0f;  // convert to As

    self.last_calibration_tick = SystemTime_GetTick();
}
*/

static void _calibrateCapacity(BmsCellModel cell_model, SocData* data, uint16_t ocv,
                               int16_t temperature) {
    float capacity = 0.0f;

    switch (cell_model) {
        case BMS_CELL_MODEL_LGES_INR21700_M50LT:
        case BMS_CELL_MODEL_SKON_E600:
            capacity =
                SocNcm_CalculateCapacityRatioByTemperature(temperature) * self.nominal_capacity;
            break;

        case BMS_CELL_MODEL_EVE_LF105:
        case BMS_CELL_MODEL_EVE_LF150:
            capacity =
                SocLfp_CalculateCapacityRatioByTemperature(temperature) * self.nominal_capacity;
            break;

        default:
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_SOC_UNSUPPORTED_CELL_MODEL);
            return;
    }

    data->full_capacity = capacity;
    data->coulomb = (data->soc / 100.0f) * capacity * 3600.0f;  // convert to As

    self.calibration.last_capacity_calibration_tick = SystemTime_GetTick();
}

static void _calibrateOcv(BmsCellModel cell_model, SocData* data, uint16_t ocv, int16_t temperature,
                          bool is_force) {
    float soc = 0.0f;

    switch (cell_model) {
        case BMS_CELL_MODEL_LGES_INR21700_M50LT:
        case BMS_CELL_MODEL_SKON_E600:
            soc = SocNcm_CalculateSoc(temperature, ocv);
            data->soc = soc;
            data->coulomb = (soc / 100.0f) * data->full_capacity * 3600.0f;  // convert to As

            self.calibration.last_ocv_calibration_tick = SystemTime_GetTick();
            break;

        case BMS_CELL_MODEL_EVE_LF105:
        case BMS_CELL_MODEL_EVE_LF150:
            if (is_force || SocLfp_IsCalibrationRegion(temperature, ocv)) {
                soc = SocLfp_CalculateSoc(temperature, ocv);
                data->soc = soc;
                data->coulomb = (soc / 100.0f) * data->full_capacity * 3600.0f;  // convert to As

                self.calibration.last_ocv_calibration_tick = SystemTime_GetTick();
            }
            break;

        default:
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_SOC_UNSUPPORTED_CELL_MODEL);
            return;
    }
}

/*
void _track(float error, float current, bool is_relaxed, float delta_time, const SocData* real,
            SocData* report) {
    float c_rate = fabsf(current) / real->full_capacity;
    if (c_rate > TRACK_MAX_C_RATE) {
        c_rate = TRACK_MAX_C_RATE;
    }

    float k = 0.08f / (1.0f + c_rate * 5.0f);

    k *= (1.0f + 0.5f * c_rate);  // Increase gain with C-rate, max 1.5x

    float delta = error * k;
    float max_rate = is_relaxed ? 0.5f : 0.2f;  // % per second
    float max_step = max_rate * delta_time;     // % per this cycle

    if (delta > max_step) delta = max_step;
    if (delta < -max_step) delta = -max_step;

    report->soc += delta;

    // clamp
    if (report->soc < 0.0f) report->soc = 0.0f;
    if (report->soc > 100.0f) report->soc = 100.0f;

    report->coulomb = (report->soc / 100.0f) * report->full_capacity * 3600.0f;

    self.track_kp = k;
    self.track_delta = delta;
}

void _track_smooth(float error, float current, bool is_relaxed, float delta_time,
                   const SocData* real, SocData* report) {
    // Time constant based smooth tracking (더 부드러운 추적)
    static float accumulated_error = 0.0f;
    static const float TIME_CONSTANT = 60.0f;  // seconds, 더 느린 응답

    float c_rate = fabsf(current) / real->full_capacity;
    if (c_rate > TRACK_MAX_C_RATE) {
        c_rate = TRACK_MAX_C_RATE;
    }

    // 더 보수적인 gain 계산 (기존 대비 약 1/3 수준)
    float base_k = 0.025f / (1.0f + c_rate * 8.0f);

    // C-rate 영향을 줄임
    float k = base_k * (1.0f + 0.2f * c_rate);

    // 오차 누적을 통한 적분 제어 추가
    accumulated_error += error * delta_time;

    // 누적 오차 제한 (windup 방지)
    float max_accumulated = 10.0f;  // %
    if (accumulated_error > max_accumulated) accumulated_error = max_accumulated;
    if (accumulated_error < -max_accumulated) accumulated_error = -max_accumulated;

    // PI controller: P term + I term
    float proportional = error * k;
    float integral = accumulated_error * (k * 0.1f);  // 적분항은 비례항의 10%

    float delta = proportional + integral;

    // 더 엄격한 변화율 제한 (기존 대비 50% 수준)
    float max_rate = is_relaxed ? 0.25f : 0.1f;  // % per second
    float max_step = max_rate * delta_time;      // % per this cycle

    if (delta > max_step) {
        delta = max_step;
        // 제한이 걸리면 적분 오차 감소
        accumulated_error *= 0.95f;
    } else if (delta < -max_step) {
        delta = -max_step;
        accumulated_error *= 0.95f;
    }

    // 저역 필터 적용 (1차 RC 필터)
    static float filtered_delta = 0.0f;
    float alpha = delta_time / (TIME_CONSTANT + delta_time);
    filtered_delta = alpha * delta + (1.0f - alpha) * filtered_delta;

    report->soc += filtered_delta;

    // clamp
    if (report->soc < 0.0f) report->soc = 0.0f;
    if (report->soc > 100.0f) report->soc = 100.0f;

    report->coulomb = (report->soc / 100.0f) * report->full_capacity * 3600.0f;

    // 디버깅용 변수 업데이트
    self.track_kp = k;
    self.track_ki = integral;
    self.track_delta = filtered_delta;

    // 오차가 충분히 작아지면 적분 오차 리셋
    if (fabsf(error) < TRACK_DEADBAND * 0.5f) {
        accumulated_error *= 0.9f;
    }
}
*/

void _track_slow(float error, float current, bool is_relaxed, float delta_time, const SocData* real,
                 SocData* report) {
    bool is_real_higher = error > 0.0f;

    if (is_relaxed) {
        report->soc += (is_real_higher ? self.config.min_track_unit : -self.config.min_track_unit);
    } else {
        // float c_rate = fabsf(current) / real->full_capacity;
        // if (c_rate > TRACK_MAX_C_RATE) {
        //     c_rate = TRACK_MAX_C_RATE;
        // }

        // dynamic correction factor
        // float unit = self.config.min_track_unit * (1.0f + (c_rate));
        float unit = (is_real_higher ? self.config.min_track_unit * TRACK_HIGH_RATE
                                     : -self.config.min_track_unit * TRACK_LOW_RATE);
        report->soc += unit;
    }

    if (report->soc < 0.0f) {
        report->soc = 0.0f;
    } else if (report->soc > 100.0f) {
        report->soc = 100.0f;
    }

    report->coulomb = (report->soc / 100.0f) * report->full_capacity * 3600.0f;
}

static void _calculateCycle(float current, float delta_time) {
    if (current >= CYCLE_CHARGE_CURRENT_THRESHOLD) {
        self.cycle.charged_coulomb += current * delta_time;                  // convert to coulomb
        self.cycle.charged_capacity = self.cycle.charged_coulomb / 3600.0f;  // convert to Ah

        self.cycle.charged_time += delta_time;

        if (self.cycle.charged_time > 0.0f && self.nominal_capacity > 0.0f) {
            float average = self.cycle.charged_coulomb / self.cycle.charged_time;  // A
            self.cycle.charged_c_rate = average / self.nominal_capacity;

        } else {
            self.cycle.charged_c_rate = 0.0f;
        }

        self.cycle.was_charging = true;
    } else {
        if (self.cycle.was_charging && self.nominal_capacity > 0.0f) {
            self.cycle.cycle += self.cycle.charged_capacity / self.nominal_capacity;
        }

        self.cycle.charged_coulomb = 0.0f;
        self.cycle.charged_capacity = 0.0f;
        self.cycle.charged_c_rate = 0.0f;
        self.cycle.charged_time = 0.0f;
        self.cycle.was_charging = false;
    }
}

AppResult Soc_Initialize(BmsConfig* config, uint16_t previous_soc, uint16_t previous_cycle,
                         uint16_t previous_soh, int16_t ocv, int16_t temperature,
                         bool is_force_calibration) {
    if (config == NULL || config->pack.parallel_count == 0 || config->pack.serial_count == 0 ||
        config->soc.track_unit == 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_SOC_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    float cell_nominal_full_capacity = BmsCellModel_GetCapacity(config->pack.cell_model);
    float pack_nominal_full_capacity =
        cell_nominal_full_capacity * (float)config->pack.parallel_count;

    self.nominal_capacity = pack_nominal_full_capacity;
    self.config.cell_model = config->pack.cell_model;
    self.config.min_track_unit = (float)config->soc.track_unit / 10.0f;   // convert to % per second
    self.config.ocv_calibration_tick = config->soc.ocv_calibration_tick;  // ms
    self.config.capacity_calibration_tick = config->soc.capacity_calibration_tick;  // ms
    self.config.track_tick = config->soc.track_tick;                                // ms

    self.real_data = (SocData){0};

    _calibrateCapacity(self.config.cell_model, &self.real_data, ocv, temperature);
    _calibrateOcv(self.config.cell_model, &self.real_data, ocv, temperature, is_force_calibration);

    bool is_ocv_calibrated = self.calibration.last_ocv_calibration_tick != 0;
    if (!is_ocv_calibrated) {
        self.real_data.soc = (float)previous_soc / 10.0f;  // convert from 0.1% to %
        self.real_data.coulomb =
            (self.real_data.soc / 100.0f) * self.real_data.full_capacity * 3600.0f;
    }

    self.report_data.soc = (float)previous_soc / 10.0f;  // convert from 0.1% to %
    self.report_data.full_capacity = self.real_data.full_capacity;
    self.report_data.coulomb =
        (self.report_data.soc / 100.0f) * self.report_data.full_capacity * 3600.0f;

    self.real_report_diff_soc = self.real_data.soc - self.report_data.soc;

    self.coulomb_data.soc = self.real_data.soc;
    self.coulomb_data.full_capacity = self.nominal_capacity;
    self.coulomb_data.coulomb =
        (self.coulomb_data.soc / 100.0f) * self.coulomb_data.full_capacity * 3600.0f;

    self.cycle.charged_coulomb = 0.0f;
    self.cycle.charged_capacity = 0.0f;
    self.cycle.charged_c_rate = 0.0f;
    self.cycle.charged_time = 0.0f;
    self.cycle.was_charging = false;

    return APP_RESULT_OK;
}

AppResult Soc_Calculate(BmsData* data, BmsSocData* soc_data) {
    if (data == NULL || soc_data == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_SOC_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    SocData real_temp = {0};
    SocData report_temp = self.report_data;
    SocData coulomb_temp = self.coulomb_data;

    uint32_t current_tick = SystemTime_GetTick();

    uint32_t delta_tick = current_tick - self.last_calculate_tick;
    float delta_time = delta_tick / 1000.0f;                          // convert to seconds
    float current = data->current.value / 10.0f;                      // convert to A
    float coulomb = current * delta_time;                             // convert to coulomb
    float capacity_coulomb = self.real_data.full_capacity * 3600.0f;  // convert to coulomb

    _calculateCycle(current, delta_time);

    real_temp.coulomb = self.real_data.coulomb + coulomb;
    real_temp.full_capacity = self.real_data.full_capacity;

    if (real_temp.coulomb < 0.0f) {
        real_temp.coulomb = 0.0f;
    }

    real_temp.soc = (real_temp.coulomb / capacity_coulomb) * 100.0f;  // convert to %

    report_temp.coulomb = self.report_data.coulomb + coulomb;
    if (report_temp.coulomb < 0.0f) {
        report_temp.coulomb = 0.0f;
    }

    float report_capacity_coulomb = report_temp.full_capacity * 3600.0f;
    report_temp.soc = (report_temp.coulomb / report_capacity_coulomb) * 100.0f;

    coulomb_temp.coulomb += coulomb;
    if (coulomb_temp.coulomb < 0.0f) {
        coulomb_temp.coulomb = 0.0f;
    }

    coulomb_temp.full_capacity = self.nominal_capacity;
    float coulomb_capacity_coulomb = coulomb_temp.full_capacity * 3600.0f;
    coulomb_temp.soc = (coulomb_temp.coulomb / coulomb_capacity_coulomb) * 100.0f;

    if (real_temp.soc < 0.0) {
        real_temp.soc = 0.0;
    } else if (real_temp.soc > 100.0) {
        real_temp.soc = 100.0;
    }

    if (report_temp.soc < 0.0) {
        report_temp.soc = 0.0;
    } else if (report_temp.soc > 100.0) {
        report_temp.soc = 100.0;
    }

    if (coulomb_temp.soc < 0.0f) {
        coulomb_temp.soc = 0.0f;
    } else if (coulomb_temp.soc > 100.0f) {
        coulomb_temp.soc = 100.0f;
    }

    uint16_t ocv = data->voltage.average_bank_voltage;                 // mV
    int16_t temperature = data->pack_temperature.average_temperature;  // 0.1°C
    BmsPackState pack_state = data->pack_state;

    bool is_relaxed = pack_state == BMS_PACK_STATE_IDLE || pack_state == BMS_PACK_STATE_SLEEP ||
                      pack_state == BMS_PACK_STATE_PROTECTION ||
                      pack_state == BMS_PACK_STATE_PERMANENT_FAILURE;

    if (is_relaxed) {
        self.calibration.capacity_calibration_tick_count += delta_tick;
        self.calibration.ocv_calibration_tick_count += delta_tick;

        if (self.calibration.capacity_calibration_tick_count >=
            self.config.capacity_calibration_tick) {
            _calibrateCapacity(self.config.cell_model, &real_temp, ocv, temperature);
            self.calibration.capacity_calibration_tick_count = 0;
        }

        if (self.calibration.ocv_calibration_tick_count >= self.config.ocv_calibration_tick) {
            _calibrateOcv(self.config.cell_model, &real_temp, ocv, temperature, false);
            self.calibration.ocv_calibration_tick_count = 0;
        }

    } else {
        self.calibration.ocv_calibration_tick_count = 0;
        self.calibration.capacity_calibration_tick_count = 0;
    }

    bool is_charge = pack_state == BMS_PACK_STATE_CHARGE ||
                     pack_state == BMS_PACK_STATE_CHARGE_CC ||
                     pack_state == BMS_PACK_STATE_CHARGE_CV;

    if (is_charge && _isFullCharge(self.config.cell_model, ocv, temperature)) {
        self.calibration.full_charge_clmap_tick_count += delta_tick;
        if (self.calibration.full_charge_clmap_tick_count >=
            LFP_FULL_CHARGE_REGION_THRESHOLD_TICK) {
            real_temp.soc = 100.0f;
            real_temp.coulomb = real_temp.full_capacity * 3600.0f;
            self.calibration.full_charge_clmap_tick_count = 0;
        }
    } else {
        self.calibration.full_charge_clmap_tick_count = 0;
    }

    float soc_diff = real_temp.soc - report_temp.soc;
    bool is_error = fabsf(soc_diff) > TRACK_DEADBAND;

    if (is_error) {
        // Use smooth tracking for more gradual SOC adjustments
        // Comment out one of the following lines to choose tracking method:

        // Fast tracking (original method)
        // _track(soc_diff, current, is_relaxed, delta_time, &real_temp, &report_temp);

        // Smooth tracking (new method - more gradual changes)
        // _track_smooth(soc_diff, current, is_relaxed, delta_time, &real_temp, &report_temp);

        // Adaptive slow tracking (new method - adaptive correction based on C-rate)
        self.track_tick_count += delta_tick;
        if (self.track_tick_count >= self.config.track_tick) {
            _track_slow(soc_diff, current, is_relaxed, delta_time, &real_temp, &report_temp);
            self.track_tick_count = 0;
        }
    } else {
        self.track_tick_count = 0;
        report_temp = real_temp;
    }

    self.real_data = real_temp;
    self.report_data = report_temp;
    self.coulomb_data = coulomb_temp;
    self.real_report_diff_soc = soc_diff;

    bool is_pack_state_changed = pack_state != self.last_pack_state;

    self.remaining_check_tick += delta_tick;

    if (is_pack_state_changed || self.remaining_check_tick >= REMAINING_TIME_CHECK_INTERVAL) {
        if (is_charge && current > 0.0f) {
            float remaining_capacity =
                self.real_data.full_capacity * (1.0f - self.real_data.soc / 100.0f);  // Ah
            self.charge_remaining_time =
                (uint32_t)((remaining_capacity / current) * 3600000.0f);  // ms
        } else {
            self.charge_remaining_time = 0;
        }

        if (pack_state == BMS_PACK_STATE_DISCHARGE && current < 0.0f) {
            float remaining_capacity =
                self.real_data.full_capacity * (self.real_data.soc / 100.0f);  // Ah
            self.discharge_remaining_time =
                (uint32_t)((remaining_capacity / -current) * 3600000.0f);  // ms
        } else {
            self.discharge_remaining_time = 0;
        }

        self.remaining_check_tick = 0;
    } else {
        // Between corrections, keep counting down in real time instead of holding the last value.
        if (is_charge && current > 0.0f) {
            self.charge_remaining_time = (self.charge_remaining_time > delta_tick)
                                             ? (self.charge_remaining_time - delta_tick)
                                             : 0;
        } else {
            self.charge_remaining_time = 0;
        }

        if (pack_state == BMS_PACK_STATE_DISCHARGE && current < 0.0f) {
            self.discharge_remaining_time = (self.discharge_remaining_time > delta_tick)
                                                ? (self.discharge_remaining_time - delta_tick)
                                                : 0;
        } else {
            self.discharge_remaining_time = 0;
        }
    }

    self.last_pack_state = pack_state;

    soc_data->real_soc = (uint16_t)(self.real_data.soc * 10.0f);  // to 0.1% units

    switch (self.config.cell_model) {
        case BMS_CELL_MODEL_SKON_E600:
        case BMS_CELL_MODEL_LGES_INR21700_M50LT: {
            // use real SOC for reporting for NCM cells
            soc_data->display_soc = (uint16_t)(self.real_data.soc * 10.0f);  // to 0.1% units
            break;
        }

        case BMS_CELL_MODEL_EVE_LF150:
        case BMS_CELL_MODEL_EVE_LF105: {
            // use tracked SOC for reporting for LFP cells
            soc_data->display_soc = (uint16_t)(self.report_data.soc * 10.0f);  // to 0.1% units
            break;
        }
    }

    soc_data->coulomb_soc = (uint16_t)(self.coulomb_data.soc * 10.0f);        // to 0.1% units
    soc_data->capacity = (uint32_t)(self.real_data.full_capacity * 1000.0f);  // to mAh units
    soc_data->tick_since_calibration = current_tick - self.calibration.last_ocv_calibration_tick;

    soc_data->cycle = (uint16_t)(self.cycle.cycle * 10.0f);  // to 0.1 cycle units

    // TODO: Implement SOH calculation
    soc_data->soh = 1000;  // to 0.1% units

    self.last_calculate_tick = current_tick;

    return APP_RESULT_OK;
}

void Soc_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

float Soc_GetCycle() { return self.cycle.cycle; }

float Soc_GetSoh() {
    return self.report_data.soc;  // Placeholder: using SOC as a proxy for SOH
}

uint32_t Soc_GetChargeRemainingTime() { return self.charge_remaining_time; }

uint32_t Soc_GetDischargeRemainingTime() { return self.discharge_remaining_time; }
