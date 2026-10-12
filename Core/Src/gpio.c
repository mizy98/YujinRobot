/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OUT_BUZZER_Pin|OUT_FET_PRECHARGE_Pin|OUT_FAN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OUT_FET_DISCHARGE_Pin|OUT_LED_STATE_GREEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, OUT_LED_STATE_BLUE_Pin|OUT_LED_STATE_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OUT_FET_CHARGE_GPIO_Port, OUT_FET_CHARGE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI2_NAND_CS_GPIO_Port, SPI2_NAND_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, OUT_NAND_HOLD_Pin|OUT_NAND_WP_Pin|OUT_VOLTAGE_ENABLE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, OUT_LED_SOC_100_Pin|OUT_LED_SOC_80_Pin|OUT_LED_SOC_60_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, OUT_LED_SOC_40_Pin|OUT_LED_SOC_20_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : OUT_BUZZER_Pin OUT_FET_DISCHARGE_Pin OUT_FET_PRECHARGE_Pin OUT_FAN_Pin
                           OUT_LED_STATE_GREEN_Pin */
  GPIO_InitStruct.Pin = OUT_BUZZER_Pin|OUT_FET_DISCHARGE_Pin|OUT_FET_PRECHARGE_Pin|OUT_FAN_Pin
                          |OUT_LED_STATE_GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_LED_STATE_BLUE_Pin OUT_LED_STATE_RED_Pin OUT_LED_SOC_40_Pin OUT_LED_SOC_20_Pin */
  GPIO_InitStruct.Pin = OUT_LED_STATE_BLUE_Pin|OUT_LED_STATE_RED_Pin|OUT_LED_SOC_40_Pin|OUT_LED_SOC_20_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : OUT_FET_CHARGE_Pin */
  GPIO_InitStruct.Pin = OUT_FET_CHARGE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OUT_FET_CHARGE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI2_NAND_CS_Pin */
  GPIO_InitStruct.Pin = SPI2_NAND_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI2_NAND_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_NAND_HOLD_Pin OUT_NAND_WP_Pin OUT_VOLTAGE_ENABLE_Pin */
  GPIO_InitStruct.Pin = OUT_NAND_HOLD_Pin|OUT_NAND_WP_Pin|OUT_VOLTAGE_ENABLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : EXTI_RTC_INT_Pin */
  GPIO_InitStruct.Pin = EXTI_RTC_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(EXTI_RTC_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_LED_SOC_100_Pin OUT_LED_SOC_80_Pin OUT_LED_SOC_60_Pin */
  GPIO_InitStruct.Pin = OUT_LED_SOC_100_Pin|OUT_LED_SOC_80_Pin|OUT_LED_SOC_60_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : IN_EXTERNAL_SWITCH_Pin */
  GPIO_InitStruct.Pin = IN_EXTERNAL_SWITCH_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(IN_EXTERNAL_SWITCH_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IN_RESCUE_Pin */
  GPIO_InitStruct.Pin = IN_RESCUE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(IN_RESCUE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IN_VOLTAGE_FAULT_Pin */
  GPIO_InitStruct.Pin = IN_VOLTAGE_FAULT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IN_VOLTAGE_FAULT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : EXTI_BDI_ENABLE_Pin */
  GPIO_InitStruct.Pin = EXTI_BDI_ENABLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(EXTI_BDI_ENABLE_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
