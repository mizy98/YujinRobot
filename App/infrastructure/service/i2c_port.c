#include "i2c_port.h"

#define self (i2c_port_private)

#define I2C_TIMEOUT (100u)
#define I2C_RECOVERY_CLOCK_PULSES (16u)
#define I2C_RECOVERY_DELAY_CYCLES (256u)

static struct {
    AppErrorInfo error_info;
    osMutexId_t mutex;
} i2c_port_private;

static void _onError(I2C_HandleTypeDef* handle) { (void)handle; }

static void _delayCycles(uint32_t cycles) {
    while (cycles-- > 0u) {
        __NOP();
    }
}

static void _recoverRtcBdiI2cBus(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = I2C2_RTC_SCL_Pin | I2C2_RTC_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(I2C2_RTC_SCL_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(I2C2_RTC_SCL_GPIO_Port, I2C2_RTC_SCL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(I2C2_RTC_SDA_GPIO_Port, I2C2_RTC_SDA_Pin, GPIO_PIN_SET);
    _delayCycles(I2C_RECOVERY_DELAY_CYCLES);

    if (HAL_GPIO_ReadPin(I2C2_RTC_SDA_GPIO_Port, I2C2_RTC_SDA_Pin) == GPIO_PIN_RESET) {
        uint32_t pulse_count = I2C_RECOVERY_CLOCK_PULSES;
        while ((pulse_count-- > 0u) &&
               (HAL_GPIO_ReadPin(I2C2_RTC_SDA_GPIO_Port, I2C2_RTC_SDA_Pin) == GPIO_PIN_RESET)) {
            HAL_GPIO_WritePin(I2C2_RTC_SCL_GPIO_Port, I2C2_RTC_SCL_Pin, GPIO_PIN_RESET);
            _delayCycles(I2C_RECOVERY_DELAY_CYCLES);
            HAL_GPIO_WritePin(I2C2_RTC_SCL_GPIO_Port, I2C2_RTC_SCL_Pin, GPIO_PIN_SET);
            _delayCycles(I2C_RECOVERY_DELAY_CYCLES);
        }
    }

    HAL_GPIO_WritePin(I2C2_RTC_SDA_GPIO_Port, I2C2_RTC_SDA_Pin, GPIO_PIN_RESET);
    _delayCycles(I2C_RECOVERY_DELAY_CYCLES);
    HAL_GPIO_WritePin(I2C2_RTC_SCL_GPIO_Port, I2C2_RTC_SCL_Pin, GPIO_PIN_SET);
    _delayCycles(I2C_RECOVERY_DELAY_CYCLES);
    HAL_GPIO_WritePin(I2C2_RTC_SDA_GPIO_Port, I2C2_RTC_SDA_Pin, GPIO_PIN_SET);
    _delayCycles(I2C_RECOVERY_DELAY_CYCLES);

    GPIO_InitStruct.Pin = I2C2_RTC_SCL_Pin | I2C2_RTC_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;

    HAL_GPIO_Init(I2C2_RTC_SCL_GPIO_Port, &GPIO_InitStruct);
}

AppResult I2cPort_Initialize(void) {
    if (self.mutex == NULL) {
        self.mutex = osMutexNew(NULL);
        if (self.mutex == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_I2C_PORT_MUTEX_CREATE);
            return APP_RESULT_FAIL;
        }
    }

    if (osMutexAcquire(self.mutex, osWaitForever) != osOK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_I2C_PORT_MUTEX_ACQUIRE);
        return APP_RESULT_FAIL;
    }

    if (HAL_I2C_GetState(I2C_RTC_BDI) != HAL_I2C_STATE_READY) {
        if (HAL_I2C_DeInit(I2C_RTC_BDI) != HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_I2C_PORT_HAL_DEINIT);
            return APP_RESULT_FAIL;
        }
    }

    if (HAL_I2C_Init(I2C_RTC_BDI) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_I2C_PORT_HAL_INIT);
        return APP_RESULT_FAIL;
    }

    (void)HAL_I2C_RegisterCallback(I2C_RTC_BDI, HAL_I2C_ERROR_CB_ID, _onError);

    if (osMutexRelease(self.mutex) != osOK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_I2C_PORT_MUTEX_RELEASE);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult I2cPort_Deinitialize(void) {
    if (self.mutex == NULL) {
        return APP_RESULT_OK;
    }

    if (osMutexAcquire(self.mutex, osWaitForever) != osOK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_I2C_PORT_MUTEX_ACQUIRE);
        return APP_RESULT_FAIL;
    }

    if (HAL_I2C_GetState(I2C_RTC_BDI) != HAL_I2C_STATE_RESET) {
        if (HAL_I2C_DeInit(I2C_RTC_BDI) != HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_I2C_PORT_HAL_DEINIT);
            return APP_RESULT_FAIL;
        }
    }

    // force SCL pulses and STOP to release a slave that keeps SDA low
    _recoverRtcBdiI2cBus();

    if (osMutexRelease(self.mutex) != osOK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_I2C_PORT_MUTEX_RELEASE);
        return APP_RESULT_FAIL;
    }

    if (self.mutex != NULL) {
        (void)osMutexDelete(self.mutex);
        self.mutex = NULL;
    }

    return APP_RESULT_OK;
}

void I2cPort_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

bool I2cPort_IsInitialized(void) { return self.mutex != NULL; }

int I2cPort_Write(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                  uint16_t length) {
    if (osMutexAcquire(self.mutex, osWaitForever) != osOK) {
        return -1;
    }

    if (HAL_I2C_Mem_Write(I2C_RTC_BDI, device_address, register_address, I2C_MEMADD_SIZE_8BIT,
                          (uint8_t*)data, length, I2C_TIMEOUT) != HAL_OK) {
        (void)osMutexRelease(self.mutex);
        return -1;
    }

    if (osMutexRelease(self.mutex) != osOK) {
        return -1;
    }

    return 0;
}

int I2cPort_Read(uint8_t device_address, uint8_t register_address, uint8_t* data, uint16_t length) {
    if (osMutexAcquire(self.mutex, osWaitForever) != osOK) {
        return -1;
    }

    if (HAL_I2C_Mem_Read(I2C_RTC_BDI, device_address, register_address, I2C_MEMADD_SIZE_8BIT, data,
                         length, I2C_TIMEOUT) != HAL_OK) {
        (void)osMutexRelease(self.mutex);
        return -1;
    }

    if (osMutexRelease(self.mutex) != osOK) {
        return -1;
    }

    return 0;
}
