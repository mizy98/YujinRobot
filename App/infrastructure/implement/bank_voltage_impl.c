#include "../../domain/interface/bank_voltage.h"
#include "../../domain/interface/rtos.h"
#include "../../domain/interface/signal.h"
#include "../../domain/service/ntc_thermistor.h"
#include "../../lib/drv_bq79616/drv_bq79616.h"
#include "../service/platform.h"

// TODO: need to fix driver UART interface

#define self (bank_voltage_private)

#define DEVICE_ADDRESS (0x00)

CCMRAM static struct {
    AppErrorInfo error_info;

    bool is_uart_tx_running;
    bool is_uart_rx_running;

    struct {
        uint8_t serial_cell_count;
    } config;

    DrvBq79616MainAdcValue adc_value;
} bank_voltage_private;

static void _onTxComplete(UART_HandleTypeDef* huart) { self.is_uart_tx_running = false; }

static void _onRxComplete(UART_HandleTypeDef* huart) { self.is_uart_rx_running = false; }

static void _onError(UART_HandleTypeDef* huart) {
    self.is_uart_tx_running = false;
    self.is_uart_rx_running = false;
}

static bool _isUartTxReady() { return !self.is_uart_tx_running; }

static bool _isUartRxReady() { return !self.is_uart_rx_running; }

static void _switchToGpio(uint8_t tx_pin_state, uint8_t rx_pin_state) {
    GPIO_InitTypeDef gpio_config = {0};

    (void)HAL_UART_Abort(UART_BANK_VOLTAGE);
    (void)HAL_UART_DeInit(UART_BANK_VOLTAGE);

    gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_config.Pull = GPIO_NOPULL;
    gpio_config.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    gpio_config.Pin = UART2_VOLTAGE_TX_Pin;
    HAL_GPIO_WritePin(UART2_VOLTAGE_TX_GPIO_Port, UART2_VOLTAGE_TX_Pin, tx_pin_state);
    HAL_GPIO_Init(UART2_VOLTAGE_TX_GPIO_Port, &gpio_config);

    gpio_config.Pin = UART2_VOLTAGE_RX_Pin;
    HAL_GPIO_WritePin(UART2_VOLTAGE_RX_GPIO_Port, UART2_VOLTAGE_RX_Pin, rx_pin_state);
    HAL_GPIO_Init(UART2_VOLTAGE_RX_GPIO_Port, &gpio_config);
}

static void _switchToUart(void) {
    HAL_GPIO_DeInit(UART2_VOLTAGE_TX_GPIO_Port, UART2_VOLTAGE_TX_Pin);
    HAL_GPIO_DeInit(UART2_VOLTAGE_RX_GPIO_Port, UART2_VOLTAGE_RX_Pin);

    (void)HAL_UART_Init(UART_BANK_VOLTAGE);

    (void)HAL_UART_RegisterCallback(UART_BANK_VOLTAGE, HAL_UART_TX_COMPLETE_CB_ID, _onTxComplete);
    (void)HAL_UART_RegisterCallback(UART_BANK_VOLTAGE, HAL_UART_RX_COMPLETE_CB_ID, _onRxComplete);
    (void)HAL_UART_RegisterCallback(UART_BANK_VOLTAGE, HAL_UART_ERROR_CB_ID, _onError);
}

static DrvBq79616Result _writeUart(uint8_t* datas, uint8_t length) {
    if (HAL_UART_Transmit_DMA(UART_BANK_VOLTAGE, datas, length) == HAL_OK) {
        self.is_uart_tx_running = true;
        return DRV_BQ79616_RESULT_OK;
    }

    return DRV_BQ79616_RESULT_FAIL;
}

static DrvBq79616Result _listenUart(uint8_t* datas, uint8_t length) {
    if (HAL_UART_Receive_DMA(UART_BANK_VOLTAGE, datas, length) == HAL_OK) {
        self.is_uart_rx_running = true;
        return DRV_BQ79616_RESULT_OK;
    }

    return DRV_BQ79616_RESULT_FAIL;
}

static void _writeTxPin(uint8_t pin_state) {
    HAL_GPIO_WritePin(UART2_VOLTAGE_TX_GPIO_Port, UART2_VOLTAGE_TX_Pin, pin_state);
}

static void _writeRxPin(uint8_t pin_state) {
    HAL_GPIO_WritePin(UART2_VOLTAGE_RX_GPIO_Port, UART2_VOLTAGE_RX_Pin, pin_state);
}

static void _delayUs(uint32_t us) {
    for (; us != 0; us--) {
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
    }
}

static void _delayCycle(void) { (void)osThreadYield(); }

static void _resetDriver(void) {
    HAL_GPIO_WritePin(OUT_VOLTAGE_ENABLE_GPIO_Port, OUT_VOLTAGE_ENABLE_Pin, GPIO_PIN_RESET);
    Rtos_SleepMs(50);

    HAL_GPIO_WritePin(OUT_VOLTAGE_ENABLE_GPIO_Port, OUT_VOLTAGE_ENABLE_Pin, GPIO_PIN_SET);
    Rtos_SleepMs(50);

    _switchToGpio(GPIO_PIN_SET, GPIO_PIN_SET);
    _writeTxPin(GPIO_PIN_RESET);

    Rtos_SleepMs(DRV_BQ79616_RESET_DELAY_MS);

    _writeTxPin(GPIO_PIN_SET);
    _switchToUart();

    Rtos_SleepMs(DRV_BQ79616_INITIAL_DELAY_MS);
}

static AppResult _initializeDriver(void) {
    DrvBq79616Result status = DRV_BQ79616_RESULT_OK;
    DrvBq79616Interface interface = {
        .isUartTxReady = _isUartTxReady,
        .isUartRxReady = _isUartRxReady,
        .writeUart = _writeUart,
        .listenUart = _listenUart,
        .switchUartToGpio = _switchToGpio,
        .switchGpioToUart = _switchToUart,
        .writeTxPin = _writeTxPin,
        .writeRxPin = _writeRxPin,
        .delayMs = Rtos_SleepMs,
        .delayUs = _delayUs,
        .delayCycle = _delayCycle,
    };

    DrvBq79616Config config = {
        .active_cell_count = VOLTAGE_MAX_COUNT,
        .is_voltage_protector_enabled = false,
        .is_temperature_protector_enabled = false,
        .under_voltage_protector_threshold = DRV_BQ79616_MIN_UV_THRESHOLD,
        .over_voltage_protector_threshold = DRV_BQ79616_MAX_OV_THRESHOLD,
        .lpf_option = DRV_BQ79616_LPF_CUT_OFF_26_HZ,
        .gpio_roles[0] = DRV_BQ79616_GPIO_ROLE_ADC_ONLY,
        .gpio_roles[2] = DRV_BQ79616_GPIO_ROLE_ADC_ONLY,
        .gpio_roles[3] = DRV_BQ79616_GPIO_ROLE_ADC_ONLY,
        .gpio_roles[5] = DRV_BQ79616_GPIO_ROLE_ADC_ONLY,
    };

    status = drv_bq79616.initialize(&interface, 1);
    if (status != DRV_BQ79616_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BANK_VOLTAGE_DRV_INIT);
        return APP_RESULT_FAIL;
    }

    status = drv_bq79616.setConfiguration(DEVICE_ADDRESS, &config);
    if (status != DRV_BQ79616_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BANK_VOLTAGE_DRV_INIT);
        return APP_RESULT_FAIL;
    }

    status = drv_bq79616.startAdc(DEVICE_ADDRESS, false, true);
    if (status != DRV_BQ79616_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BANK_VOLTAGE_DRV_ADC_START);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult BankVoltage_Initialize(const BmsConfig* config) {
    Signal_SetVoltageEnable(true);

    Rtos_SleepMs(100);

    if (HAL_UART_GetState(UART_BANK_VOLTAGE) == HAL_UART_STATE_RESET) {
        if (HAL_UART_Init(UART_BANK_VOLTAGE) != HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BANK_VOLTAGE_HAL_INIT);
            return APP_RESULT_FAIL;
        }
    }

    (void)HAL_UART_RegisterCallback(UART_BANK_VOLTAGE, HAL_UART_TX_COMPLETE_CB_ID, _onTxComplete);
    (void)HAL_UART_RegisterCallback(UART_BANK_VOLTAGE, HAL_UART_RX_COMPLETE_CB_ID, _onRxComplete);
    (void)HAL_UART_RegisterCallback(UART_BANK_VOLTAGE, HAL_UART_ERROR_CB_ID, _onError);

    self.config.serial_cell_count = config->pack.serial_count;

    _resetDriver();

    if (_initializeDriver() != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    osDelay(200);

    self.error_info = (AppErrorInfo){0};

    return APP_RESULT_OK;
}

AppResult BankVoltage_DeInitialize(void) {
    if (HAL_UART_DeInit(UART_BANK_VOLTAGE) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BANK_VOLTAGE_HAL_DEINIT);
        return APP_RESULT_FAIL;
    }

    self.is_uart_tx_running = false;
    self.is_uart_rx_running = false;

    return APP_RESULT_OK;
}

void BankVoltage_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult BankVoltage_GetData(BmsVoltageData* data, BmsBalancerTemperatureData* temperature_data) {
    DrvBq79616Result result = DRV_BQ79616_RESULT_OK;
    DrvBq79616MainAdcValue value = {0};

    result = drv_bq79616.getMainAdcValue(DEVICE_ADDRESS, &value);
    if (result != DRV_BQ79616_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BANK_VOLTAGE_DRV_ADC_READ);
        return APP_RESULT_FAIL;
    }

    self.adc_value = value;

    uint32_t total_voltage = 0;         // mV
    uint16_t max_voltage = 0;           // mV
    uint16_t min_voltage = UINT16_MAX;  // mV
    uint8_t max_voltage_index = 0;
    uint8_t min_voltage_index = 0;

    for (uint8_t i = 0; i < self.config.serial_cell_count; i++) {
        uint16_t converted_voltage = (uint16_t)(value.cell_voltages[i] * 1000);  // Convert V to mV

        data->bank_voltages[i].voltage = converted_voltage;
        data->bank_voltages[i].is_balancing = 0;  // TODO: need to implement balancing status

        total_voltage += converted_voltage;

        if (converted_voltage > max_voltage) {
            max_voltage = converted_voltage;
            max_voltage_index = i;
        }

        if (converted_voltage < min_voltage) {
            min_voltage = converted_voltage;
            min_voltage_index = i;
        }
    }

    data->pack_voltage = (uint16_t)(total_voltage / 100);  // Convert mV to 0.1V
    data->max_bank_voltage = max_voltage;
    data->min_bank_voltage = min_voltage;
    data->average_bank_voltage = (uint16_t)(total_voltage / self.config.serial_cell_count);
    data->dv = max_voltage - min_voltage;                   // Voltage difference
    data->max_voltage_bank_number = max_voltage_index + 1;  // 1-based numbering
    data->min_voltage_bank_number = min_voltage_index + 1;  // 1-based numbering

    if (temperature_data != NULL) {
        temperature_data->temperatures[0] =
            (int16_t)(value.die_temperature * 10);  // Convert °C to 0.1°C
        temperature_data->max_temperature = temperature_data->temperatures[0];
        temperature_data->min_temperature = temperature_data->temperatures[0];
        temperature_data->average_temperature = temperature_data->temperatures[0];
        temperature_data->dt = 0;
        temperature_data->max_temperature_number = 1;
        temperature_data->min_temperature_number = 1;
    }

    return APP_RESULT_OK;
}

AppResult BankVoltage_GetInputVoltage(uint32_t* data) {
    // only use cached value, do not read from driver to avoid blocking

    double raw_voltage = (self.adc_value.gpio_voltages[3]  // raw voltage in V
                          * 0.75                           // TPS37100W41DDYYR scale
                          * 41)  // (R1 + R2) / R2, where R2 = 2MΩ, R2 = 25kΩ
                         + 0.1;  // diode forward voltage drop + error compensation;

    *data = (uint32_t)(raw_voltage * 1000.0);  // Convert V to mV

    return APP_RESULT_OK;
}

AppResult BankVoltage_GetContactorTemperatureData(BmsContactorTemperatureData* data) {
    // only use cached value, do not read from driver to avoid blocking

    uint16_t adc = self.adc_value.gpio_voltages[5]  // raw voltage in V
                   / 5.0                            // adc reference voltage
                   * 25840.0;                       // adc resolution (@ref temperature_impl.c)
    float raw_temperature = NtcThermistor_GetTemperature(NTC_THERMISTOR_TYPE_10K_3630, adc);

    data->temperatures[0] = (int16_t)(raw_temperature * 10.0f);  // Convert °C to 0.1°C

    // only one contactor temperature sensor, so max, min, average are the same
    data->max_temperature = data->temperatures[0];
    data->min_temperature = data->temperatures[0];
    data->average_temperature = data->temperatures[0];
    data->dt = 0;
    data->max_temperature_number = 1;
    data->min_temperature_number = 1;

    return APP_RESULT_OK;
}