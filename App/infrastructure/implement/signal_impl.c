#include "../../domain/interface/signal.h"
#include "../service/platform.h"

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    GPIO_PinState active_state;
    TimerHandle_t timer;
    void (*callback)(void);
} ExtiHandle;

CCMRAM static struct {
    ExtiHandle exti[16];
} signal_private;

static void _processDebounce(TimerHandle_t timer) {
    ExtiHandle* handle = (ExtiHandle*)pvTimerGetTimerID(timer);

    if (handle->callback != NULL &&
        HAL_GPIO_ReadPin(handle->port, handle->pin) == handle->active_state) {
        handle->callback();
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    ExtiHandle* handle = &signal_private.exti[POSITION_VAL(pin)];

    if (handle->timer != NULL) {
        BaseType_t higher_priority_task_woken = pdFALSE;
        xTimerResetFromISR(handle->timer, &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}

static void _registerExtiCallback(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState active_state,
                                  void (*callback)(void), uint32_t debounce_tick) {
    ExtiHandle* handle = &signal_private.exti[POSITION_VAL(pin)];

    if (handle->timer != NULL) {
        xTimerDelete(handle->timer, portMAX_DELAY);
        handle->timer = NULL;
    }

    handle->port = port;
    handle->pin = pin;
    handle->active_state = active_state;
    handle->callback = callback;

    if (callback != NULL && debounce_tick > 0) {
        handle->timer = xTimerCreate("check_debounce", pdMS_TO_TICKS(debounce_tick), pdFALSE,
                                     handle, _processDebounce);
    }
}

static void _disableExtiCallback(uint16_t pin) {
    ExtiHandle* handle = &signal_private.exti[POSITION_VAL(pin)];

    if (handle->timer != NULL) {
        xTimerDelete(handle->timer, portMAX_DELAY);
        handle->timer = NULL;
    }

    handle->callback = NULL;
}

bool Signal_IsRescueOn(void) {
    return HAL_GPIO_ReadPin(IN_RESCUE_GPIO_Port, IN_RESCUE_Pin) == GPIO_PIN_SET;
}

bool Signal_IsBdiEnableOn(void) {
    return HAL_GPIO_ReadPin(EXTI_BDI_ENABLE_GPIO_Port, EXTI_BDI_ENABLE_Pin) == GPIO_PIN_SET;
}

void Signal_RegisterBdiEnableOnCallback(void (*callback)(void), uint32_t debounce_tick) {
    _registerExtiCallback(EXTI_BDI_ENABLE_GPIO_Port, EXTI_BDI_ENABLE_Pin, GPIO_PIN_SET, callback,
                          debounce_tick);
}

void Signal_DisableBdiEnableOnCallback(void) { _disableExtiCallback(EXTI_BDI_ENABLE_Pin); }

void Signal_SetVoltageEnable(bool state) {
    HAL_GPIO_WritePin(OUT_VOLTAGE_ENABLE_GPIO_Port, OUT_VOLTAGE_ENABLE_Pin,
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Signal_SetChargePrechargeContactor(bool state) {
    // HAL_GPIO_WritePin(OUT_FET_PRECHARGE_GPIO_Port, OUT_FET_PRECHARGE_Pin,
    //                   state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Signal_SetChargeContactor(bool state) {
    HAL_GPIO_WritePin(OUT_FET_CHARGE_GPIO_Port, OUT_FET_CHARGE_Pin,
                      state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Signal_SetDischargeContactor(bool state) {
    HAL_GPIO_WritePin(OUT_FET_DISCHARGE_GPIO_Port, OUT_FET_DISCHARGE_Pin,
                      state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Signal_SetBuzzer(bool state) {
    HAL_GPIO_WritePin(OUT_BUZZER_GPIO_Port, OUT_BUZZER_Pin, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Signal_SetSocInternalIndicator1(bool state) {
    HAL_GPIO_WritePin(OUT_LED_SOC_20_GPIO_Port, OUT_LED_SOC_20_Pin,
                      state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Signal_SetSocInternalIndicator2(bool state) {
    HAL_GPIO_WritePin(OUT_LED_SOC_40_GPIO_Port, OUT_LED_SOC_40_Pin,
                      state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Signal_SetSocInternalIndicator3(bool state) {
    HAL_GPIO_WritePin(OUT_LED_SOC_60_GPIO_Port, OUT_LED_SOC_60_Pin,
                      state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Signal_SetSocInternalIndicator4(bool state) {
    HAL_GPIO_WritePin(OUT_LED_SOC_80_GPIO_Port, OUT_LED_SOC_80_Pin,
                      state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Signal_SetSocInternalIndicator5(bool state) {
    HAL_GPIO_WritePin(OUT_LED_SOC_100_GPIO_Port, OUT_LED_SOC_100_Pin,
                      state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Signal_SetFan(bool state) {
    HAL_GPIO_WritePin(OUT_FAN_GPIO_Port, OUT_FAN_Pin, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
