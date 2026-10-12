#include "adc_driver.h"

#include "../../lib/drv_ads1115/drv_ads1115.h"

#define self (adc_driver_private)

static struct {
    bool is_initialized;

    osMutexId_t mutex;

    AppErrorInfo error_info;
} adc_driver_private;

static int _write(uint8_t address, uint8_t* data, uint16_t length) {
    if (HAL_I2C_Master_Transmit(I2C_CURRENT, (address << 1), data, length, 100) != HAL_OK) {
        return -1;
    }
    return 0;
}

static int _read(uint8_t address, uint8_t* data, uint16_t length) {
    if (HAL_I2C_Master_Receive(I2C_CURRENT, (address << 1), data, length, 100) != HAL_OK) {
        return -1;
    }
    return 0;
}

AppResult AdcDriver_Initialize(void) {
    if (self.mutex == NULL) {
        void* mutex = osMutexNew(NULL);
        if (mutex == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_MUTEX_CREATE);
            return APP_RESULT_FAIL;
        }
        self.mutex = mutex;
    }

    if (osMutexAcquire(self.mutex, osWaitForever) != osOK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_MUTEX_LOCK);
        return APP_RESULT_FAIL;
    }

    (void)HAL_I2C_DeInit(I2C_CURRENT);

    if (HAL_I2C_GetState(I2C_CURRENT) == HAL_I2C_STATE_RESET) {
        if (HAL_I2C_Init(I2C_CURRENT) != HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_HAL_INIT);
            osMutexRelease(self.mutex);
            return APP_RESULT_FAIL;
        }
    }

    int result = DrvAds1115_Initialize(&(DrvAds1115Interface){
        .write = _write,
        .read = _read,
        .delayMs = (void (*)(uint32_t))osDelay,
    });
    if (result != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_DRV_INIT);
        osMutexRelease(self.mutex);
        return APP_RESULT_FAIL;
    }

    self.is_initialized = true;

    if (osMutexRelease(self.mutex) != osOK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_MUTEX_UNLOCK);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult AdcDriver_Deinitialize(void) {
    if (osMutexAcquire(self.mutex, osWaitForever) != osOK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_MUTEX_LOCK);
        return APP_RESULT_FAIL;
    }

    if (HAL_I2C_DeInit(I2C_CURRENT) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_HAL_DEINIT);
        osMutexRelease(self.mutex);
        return APP_RESULT_FAIL;
    }

    self.is_initialized = false;

    if (osMutexRelease(self.mutex) != osOK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_MUTEX_UNLOCK);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

bool AdcDriver_IsInitialized(void) { return self.is_initialized; }

void AdcDriver_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult AdcDriver_Read(AdcDriverChannel channel, int16_t* value) {
    if (value == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    if (osMutexAcquire(self.mutex, osWaitForever) != osOK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_MUTEX_LOCK);
        return APP_RESULT_FAIL;
    }

    uint8_t drv_channel;

    switch (channel) {
        case ADC_CHANNEL_CURRENT:
            drv_channel = DRV_ADS1115_CHANNEL_MODE_SINGLE_ENDED_CHANNEL0;
            break;
        case ADC_CHANNEL_TEMPERATURE_0:
            drv_channel = DRV_ADS1115_CHANNEL_MODE_SINGLE_ENDED_CHANNEL1;
            break;
        case ADC_CHANNEL_TEMPERATURE_1:
            drv_channel = DRV_ADS1115_CHANNEL_MODE_SINGLE_ENDED_CHANNEL2;
            break;
        case ADC_CHANNEL_TEMPERATURE_2:
            drv_channel = DRV_ADS1115_CHANNEL_MODE_SINGLE_ENDED_CHANNEL3;
            break;
        default:
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_INVALID_ARG);
            osMutexRelease(self.mutex);
            return APP_RESULT_INVALID_ARG;
    }

    int result =
        DrvAds1115_StartConversion(drv_channel, DRV_ADS1115_DATA_SAMPLE_RATE_860_SAMPLES_PER_SECOND,
                                   DRV_ADS1115_GAIN_V4_096, true);
    if (result != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_DRV_START);
        osMutexRelease(self.mutex);
        return APP_RESULT_FAIL;
    }

    if (DrvAds1115_WaitUntilReady() != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_DRV_READ);
        osMutexRelease(self.mutex);
        return APP_RESULT_FAIL;
    }

    int16_t adc_value = 0;
    result = DrvAds1115_ReadConversion(&adc_value);
    if (result != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_DRV_READ);
        osMutexRelease(self.mutex);
        return APP_RESULT_FAIL;
    }

    *value = adc_value;

    if (osMutexRelease(self.mutex) != osOK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_ADC_DRIVER_MUTEX_UNLOCK);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

float AdcDriver_ConvertToVoltage(int16_t value) {
    return DrvAds1115_ConvertToVoltage(value, DRV_ADS1115_GAIN_V4_096);
}