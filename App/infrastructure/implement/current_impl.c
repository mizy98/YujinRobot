#include <math.h>

#include "../../domain/interface/current.h"
#include "../../domain/interface/rtos.h"
#include "../../domain/interface/signal.h"
#include "../../domain/interface/system_time.h"
#include "../../domain/model/ll/app_config.h"
#include "../../domain/service/filter.h"
#include "../service/adc_driver.h"

// CHK400B (±400 [A])
// offset voltage: 2500 [mV]
// offset error:  ±20 [mV]
// offset temperature error : ±20 [mV]
// hysteresis error: ±5 [mV]
// = 2500 + (offset error + offset temperature error + hysteresis error)

#define self (current_private)

#define CURRENT_CALIBRATION_REF_VOLTAGE (2.500f)                    // V
#define CURRENT_CALIBRATION_REF_VOLTAGE_DIFF_THRESHOLD (0.010f)     // V (10mV)
#define CURRENT_CALIBRATION_STABLE_VOLTAGE_DIFF_THRESHOLD (0.005f)  // V (5mV)
#define CURRENT_CALIBRATION_STABLE_VOLTAGE_EMA_ALPHA (0.1f)

// A fixed adc_voltage-near-2.5V check alone doesn't account for offset drift, so also require
// the actually computed current (using the last calibrated offset) to be near zero.
#define CURRENT_CALIBRATION_STABLE_CURRENT_THRESHOLD (1.0f)  // A

static struct {
    uint32_t last_adc_tick;  // ms

    float offset_adc_voltage;  // V
    float adc_voltage;         // V
    float current;             // A
    float filtered_current;    // A

    bool is_initial_calibration_done;
    struct {
        uint32_t last_calibration_tick;  // ms
        uint32_t relaxed_start_tick;     // ms

        float stable_voltage_ema;            // V
        uint32_t voltage_stable_start_tick;  // ms
    } calibration;

    struct {
        uint8_t direction;
        uint32_t calibration_tick;  // ms
        float ema_alpha;
    } config;

    AppErrorInfo error_info;
} current_private;

static float _toCurrent(float voltage, float offset_voltage) {
    if (!isfinite(voltage) || !isfinite(offset_voltage)) {
        return 0.0f;
    }

    if (CURRENT_RESOLUTION_VOLTAGE == 0.0f) {
        return 0.0f;
    }

    if (self.config.direction == 0) {
        return (voltage - offset_voltage) / CURRENT_RESOLUTION_VOLTAGE;
    } else {
        return (-1) * ((voltage - offset_voltage) / CURRENT_RESOLUTION_VOLTAGE);
    }
}

static AppResult _calibrateZeroOffset(void) {
    int16_t adc_data = 0;

    if (AdcDriver_Read(ADC_CHANNEL_CURRENT, &adc_data) != APP_RESULT_OK) {
        AdcDriver_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    self.offset_adc_voltage = AdcDriver_ConvertToVoltage(adc_data);
    self.calibration.last_calibration_tick = SystemTime_GetTick();
    self.calibration.stable_voltage_ema = self.offset_adc_voltage;
    self.calibration.voltage_stable_start_tick = 0;

    return APP_RESULT_OK;
}

static AppResult _processCalibration(float adc_voltage, BmsPackState pack_state) {
    uint32_t current_tick = SystemTime_GetTick();

    bool is_current_near_zero = fabsf(self.current) <= CURRENT_CALIBRATION_STABLE_CURRENT_THRESHOLD;

    bool is_relaxed =
        is_current_near_zero &&
        adc_voltage <
            (CURRENT_CALIBRATION_REF_VOLTAGE + CURRENT_CALIBRATION_REF_VOLTAGE_DIFF_THRESHOLD) &&
        adc_voltage >
            (CURRENT_CALIBRATION_REF_VOLTAGE - CURRENT_CALIBRATION_REF_VOLTAGE_DIFF_THRESHOLD);

    if (!is_relaxed) {
        self.calibration.voltage_stable_start_tick = 0u;
        self.calibration.stable_voltage_ema = adc_voltage;
        return APP_RESULT_OK;
    }

    if (!isfinite(self.calibration.stable_voltage_ema)) {
        self.calibration.stable_voltage_ema = adc_voltage;
    }

    self.calibration.stable_voltage_ema =
        Filter_Ema(self.calibration.stable_voltage_ema, adc_voltage,
                   CURRENT_CALIBRATION_STABLE_VOLTAGE_EMA_ALPHA);

    float ref_diff = fabsf(self.calibration.stable_voltage_ema - CURRENT_CALIBRATION_REF_VOLTAGE);
    float variation = fabsf(adc_voltage - self.calibration.stable_voltage_ema);

    bool is_near_reference = ref_diff <= CURRENT_CALIBRATION_REF_VOLTAGE_DIFF_THRESHOLD;
    bool is_voltage_stable = variation <= CURRENT_CALIBRATION_STABLE_VOLTAGE_DIFF_THRESHOLD;

    if (is_near_reference && is_voltage_stable) {
        if (self.calibration.voltage_stable_start_tick == 0u) {
            self.calibration.voltage_stable_start_tick = current_tick;
            return APP_RESULT_OK;
        }
        if ((current_tick - self.calibration.voltage_stable_start_tick) >=
            self.config.calibration_tick) {
            if (_calibrateZeroOffset() != APP_RESULT_OK) {
                return APP_RESULT_FAIL;
            }
        }
    } else {
        self.calibration.voltage_stable_start_tick = 0u;
    }

    return APP_RESULT_OK;
}

AppResult Current_Initialize(const BmsConfig* config) {
    if (config == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CURRENT_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    Signal_SetCurrentEnable(true);

    Rtos_SleepMs(100);

    self.config.direction = config->current.direction;
    self.config.calibration_tick = config->current.calibration_tick;
    self.config.ema_alpha =
        (float)config->current.ema_alpha / 1000.0f;  // convert fixed-point to float

    if (self.config.ema_alpha < 0.0f || self.config.ema_alpha > 1.0f ||
        !isfinite(self.config.ema_alpha)) {
        self.config.ema_alpha = 0.1f;
    }

    self.adc_voltage = 0.0f;
    self.current = 0.0f;
    self.filtered_current = 0.0f;
    self.last_adc_tick = 0;

    self.calibration.last_calibration_tick = 0;
    self.calibration.relaxed_start_tick = 0;
    self.calibration.voltage_stable_start_tick = 0;
    self.calibration.stable_voltage_ema = CURRENT_CALIBRATION_REF_VOLTAGE;

    if (!AdcDriver_IsInitialized()) {
        if (AdcDriver_Initialize() != APP_RESULT_OK) {
            AdcDriver_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    if (!self.is_initial_calibration_done) {
        self.offset_adc_voltage = 0.0f;
        if (_calibrateZeroOffset() != APP_RESULT_OK) {
            return APP_RESULT_FAIL;
        }
        self.is_initial_calibration_done = true;
    }

    return APP_RESULT_OK;
}

AppResult Current_Deinitialize(void) {
    if (AdcDriver_Deinitialize() != APP_RESULT_OK) {
        AdcDriver_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void Current_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult Current_GetData(BmsPackState pack_state, BmsCurrentData* data) {
    (void)pack_state;

    if (data == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CURRENT_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    uint32_t current_tick = SystemTime_GetTick();
    uint32_t elapsed_tick = current_tick - self.last_adc_tick;

    if (elapsed_tick < CURRENT_READ_TICK) {
        Rtos_SleepMs(CURRENT_READ_TICK - elapsed_tick);
    }

    int16_t adc_data = 0;

    if (AdcDriver_Read(ADC_CHANNEL_CURRENT, &adc_data) != APP_RESULT_OK) {
        AdcDriver_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    float adc_voltage = AdcDriver_ConvertToVoltage(adc_data);
    float current = _toCurrent(adc_voltage, self.offset_adc_voltage);

    if (!isfinite(current)) {
        current = 0.0f;
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_CURRENT_DRV_ADC_READ);
    }

    float filtered_current = Filter_Ema(self.filtered_current, current, self.config.ema_alpha);

    if (!isfinite(filtered_current)) {
        filtered_current = current;
    }

    self.last_adc_tick = current_tick;

    uint32_t tick_since_calibration = current_tick - self.calibration.last_calibration_tick;
    BmsCurrentData temp_data = {0};

    temp_data.value = (int16_t)(current * 10);                                  // convert to 0.1A
    temp_data.filtered_value = (int16_t)(filtered_current * 10);                // convert to 0.1A
    temp_data.adc_voltage = (uint16_t)(adc_voltage * 1000.0f);                  // convert to mV
    temp_data.adc_offset_voltage = (uint16_t)(self.offset_adc_voltage * 1000);  // convert to mV
    temp_data.tick_since_calibration = tick_since_calibration;

    *data = temp_data;

    self.adc_voltage = adc_voltage;
    self.current = current;
    self.filtered_current = filtered_current;

    if (_processCalibration(adc_voltage, pack_state) != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}
