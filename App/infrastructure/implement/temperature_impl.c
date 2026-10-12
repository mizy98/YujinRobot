#include "../../domain/interface/temperature.h"
#include "../../domain/service/ntc_thermistor.h"
#include "../service/adc_driver.h"
#include "../service/platform.h"

#define ADC_FULL_RESOLUTION (32768.0f)
#define ADC_REFERENCE_VOLTAGE (4.096f)  // ADS1115 4.096 FSR
#define DIVIDER_VOLTAGE (3.23f)

#define MAX_ADC_CHANNEL_COUNT (4u)

#define self (temperature_private)

static struct {
    AppErrorInfo error_info;

    uint32_t hal_dma_buffer[MAX_ADC_CHANNEL_COUNT];
    bool is_hal_dma_ready;
    uint32_t hal_channel_count;

    BmsThermalConfig config;
} temperature_private;

static void _onAdcCompleted(ADC_HandleTypeDef* hadc) { self.is_hal_dma_ready = true; }

AppResult Temperature_Initialize(const BmsConfig* config) {
    if (config == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TEMPERATURE_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    if (!AdcDriver_IsInitialized()) {
        if (AdcDriver_Initialize() != APP_RESULT_OK) {
            AdcDriver_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    // NTC divider is driven by 3.3V, not ADC reference voltage (4.096V).
    // Therefore, scale down the resolution used for converting resistance ratio to ADC code
    // by the actual voltage ratio.
    uint32_t adc_resoultion =
        (uint32_t)(ADC_FULL_RESOLUTION * (DIVIDER_VOLTAGE / ADC_REFERENCE_VOLTAGE));

    NtcThermistor_Initialize(TEMPERATURE_SERIAL_RESISTOR, adc_resoultion);

    self.config = config->thermal;

    (void)HAL_ADC_RegisterCallback(ADC_TEMPERATURE, HAL_ADC_CONVERSION_COMPLETE_CB_ID,
                                   _onAdcCompleted);

    uint32_t board_count = self.config.board_temperature_count > MAX_ADC_CHANNEL_COUNT
                               ? MAX_ADC_CHANNEL_COUNT
                               : self.config.board_temperature_count;
    if (HAL_ADC_Start_DMA(ADC_TEMPERATURE, (uint32_t*)self.hal_dma_buffer, board_count) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TEMPERATURE_HAL_ADC_START);
        return APP_RESULT_FAIL;
    }

    self.hal_channel_count = board_count;

    self.error_info = (AppErrorInfo){0};

    return APP_RESULT_OK;
}

AppResult Temperature_DeInitialize(void) { return APP_RESULT_OK; }

void Temperature_GetLastErrorInfo(AppErrorInfo* error_info) {
    if (error_info != NULL) {
        *error_info = self.error_info;
    }
}

AppResult Temperature_GetDatas(BmsPackTemperatureData* pack_data,
                               BmsBoardTemperatureData* board_data,
                               BmsBalancerTemperatureData* balancer_data,
                               BmsContactorTemperatureData* contactor_data) {
    int16_t adc_values[3] = {0};

    for (uint8_t i = 0; i < 3; ++i) {
        if (AdcDriver_Read(ADC_CHANNEL_TEMPERATURE_0 + i, &adc_values[i]) != APP_RESULT_OK) {
            AdcDriver_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    if (pack_data != NULL) {
        float total_temperature = 0.0f;       // 0.1℃
        int16_t max_temperature = INT16_MIN;  // 0.1℃
        int16_t min_temperature = INT16_MAX;  // 0.1℃
        uint8_t max_temperature_number = 0;
        uint8_t min_temperature_number = 0;

        for (uint8_t i = 0; i < self.config.pack_temperature_count; ++i) {
            float temperature =
                NtcThermistor_GetTemperature(NTC_THERMISTOR_TYPE_10K_3950, (uint16_t)adc_values[i]);
            int16_t converted_temperature = (int16_t)(temperature * 10.0f);

            pack_data->temperatures[i] = converted_temperature;

            total_temperature += temperature;

            if (converted_temperature > max_temperature) {
                max_temperature = converted_temperature;
                max_temperature_number = i;
            }

            if (converted_temperature < min_temperature) {
                min_temperature = converted_temperature;
                min_temperature_number = i;
            }
        }

        pack_data->max_temperature = max_temperature;
        pack_data->min_temperature = min_temperature;
        pack_data->average_temperature =
            (int16_t)(total_temperature * 10.0f / self.config.pack_temperature_count);
        pack_data->dt = max_temperature - min_temperature;
        pack_data->max_temperature_number = max_temperature_number + 1;
        pack_data->min_temperature_number = min_temperature_number + 1;
    }

    if (board_data != NULL && self.is_hal_dma_ready) {
        self.is_hal_dma_ready = false;

        int16_t max_temperature = INT16_MIN;  // 0.1℃
        int16_t min_temperature = INT16_MAX;  // 0.1℃
        uint8_t max_temperature_number = 0;
        uint8_t min_temperature_number = 0;

        for (uint8_t i = 0; i < self.hal_channel_count; ++i) {
            uint16_t adc_value = (uint16_t)self.hal_dma_buffer[i] * (ADC_FULL_RESOLUTION / 4096.0f);
            float temperature =
                NtcThermistor_GetTemperature(NTC_THERMISTOR_TYPE_10K_3630, adc_value);
            int16_t converted_temperature = (int16_t)(temperature * 10.0f);

            board_data->temperatures[i] = converted_temperature;
            if (converted_temperature > max_temperature) {
                max_temperature = converted_temperature;
                max_temperature_number = i;
            }
            if (converted_temperature < min_temperature) {
                min_temperature = converted_temperature;
                min_temperature_number = i;
            }
        }

        board_data->max_temperature = max_temperature;
        board_data->min_temperature = min_temperature;
        board_data->average_temperature = (int16_t)((max_temperature + min_temperature) / 2);
        board_data->dt = max_temperature - min_temperature;
        board_data->max_temperature_number = max_temperature_number + 1;
        board_data->min_temperature_number = min_temperature_number + 1;

        if (HAL_ADC_Start_DMA(ADC_TEMPERATURE, (uint32_t*)self.hal_dma_buffer,
                              self.hal_channel_count) != HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TEMPERATURE_HAL_ADC_START);
            return APP_RESULT_FAIL;
        }
    }

    (void)balancer_data;
    (void)contactor_data;

    return APP_RESULT_OK;
}
