/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void MX_Init(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC1_CH10_BOARD_TEMPERATURE1_Pin GPIO_PIN_0
#define ADC1_CH10_BOARD_TEMPERATURE1_GPIO_Port GPIOC
#define OUT_BUZZER_Pin GPIO_PIN_1
#define OUT_BUZZER_GPIO_Port GPIOA
#define OUT_FET_DISCHARGE_Pin GPIO_PIN_4
#define OUT_FET_DISCHARGE_GPIO_Port GPIOA
#define OUT_FET_PRECHARGE_Pin GPIO_PIN_5
#define OUT_FET_PRECHARGE_GPIO_Port GPIOA
#define OUT_FAN_Pin GPIO_PIN_6
#define OUT_FAN_GPIO_Port GPIOA
#define OUT_LED_STATE_GREEN_Pin GPIO_PIN_7
#define OUT_LED_STATE_GREEN_GPIO_Port GPIOA
#define OUT_LED_STATE_BLUE_Pin GPIO_PIN_4
#define OUT_LED_STATE_BLUE_GPIO_Port GPIOC
#define OUT_LED_STATE_RED_Pin GPIO_PIN_5
#define OUT_LED_STATE_RED_GPIO_Port GPIOC
#define OUT_FET_CHARGE_Pin GPIO_PIN_0
#define OUT_FET_CHARGE_GPIO_Port GPIOB
#define I2C2_RTC_SCL_Pin GPIO_PIN_10
#define I2C2_RTC_SCL_GPIO_Port GPIOB
#define I2C2_RTC_SDA_Pin GPIO_PIN_11
#define I2C2_RTC_SDA_GPIO_Port GPIOB
#define SPI2_NAND_CS_Pin GPIO_PIN_12
#define SPI2_NAND_CS_GPIO_Port GPIOB
#define SPI2_NAND_SCK_Pin GPIO_PIN_13
#define SPI2_NAND_SCK_GPIO_Port GPIOB
#define SPI2_NAND_MISO_Pin GPIO_PIN_14
#define SPI2_NAND_MISO_GPIO_Port GPIOB
#define SPI2_NAND_MOSI_Pin GPIO_PIN_15
#define SPI2_NAND_MOSI_GPIO_Port GPIOB
#define OUT_NAND_HOLD_Pin GPIO_PIN_8
#define OUT_NAND_HOLD_GPIO_Port GPIOD
#define OUT_NAND_WP_Pin GPIO_PIN_9
#define OUT_NAND_WP_GPIO_Port GPIOD
#define EXTI_RTC_INT_Pin GPIO_PIN_12
#define EXTI_RTC_INT_GPIO_Port GPIOD
#define OUT_LED_SOC_100_Pin GPIO_PIN_13
#define OUT_LED_SOC_100_GPIO_Port GPIOD
#define OUT_LED_SOC_80_Pin GPIO_PIN_14
#define OUT_LED_SOC_80_GPIO_Port GPIOD
#define OUT_LED_SOC_60_Pin GPIO_PIN_15
#define OUT_LED_SOC_60_GPIO_Port GPIOD
#define OUT_LED_SOC_40_Pin GPIO_PIN_6
#define OUT_LED_SOC_40_GPIO_Port GPIOC
#define OUT_LED_SOC_20_Pin GPIO_PIN_7
#define OUT_LED_SOC_20_GPIO_Port GPIOC
#define I2C3_CURRENT_SDA_Pin GPIO_PIN_9
#define I2C3_CURRENT_SDA_GPIO_Port GPIOC
#define I2C3_CURRENT_SCL_Pin GPIO_PIN_8
#define I2C3_CURRENT_SCL_GPIO_Port GPIOA
#define USART1_RS232_TX_Pin GPIO_PIN_9
#define USART1_RS232_TX_GPIO_Port GPIOA
#define USART1_RS232_RX_Pin GPIO_PIN_10
#define USART1_RS232_RX_GPIO_Port GPIOA
#define UART4_DEBUG_RS232_TX_Pin GPIO_PIN_10
#define UART4_DEBUG_RS232_TX_GPIO_Port GPIOC
#define UART4_DEBUG_RS232_RX_Pin GPIO_PIN_11
#define UART4_DEBUG_RS232_RX_GPIO_Port GPIOC
#define IN_EXTERNAL_SWITCH_Pin GPIO_PIN_12
#define IN_EXTERNAL_SWITCH_GPIO_Port GPIOC
#define IN_RESCUE_Pin GPIO_PIN_2
#define IN_RESCUE_GPIO_Port GPIOD
#define OUT_VOLTAGE_ENABLE_Pin GPIO_PIN_3
#define OUT_VOLTAGE_ENABLE_GPIO_Port GPIOD
#define IN_VOLTAGE_FAULT_Pin GPIO_PIN_4
#define IN_VOLTAGE_FAULT_GPIO_Port GPIOD
#define IN_VOLTAGE_FAULT_EXTI_IRQn EXTI4_IRQn
#define UART2_VOLTAGE_TX_Pin GPIO_PIN_5
#define UART2_VOLTAGE_TX_GPIO_Port GPIOD
#define UART2_VOLTAGE_RX_Pin GPIO_PIN_6
#define UART2_VOLTAGE_RX_GPIO_Port GPIOD
#define CAN2_EXTERNAL_COMM_RX_Pin GPIO_PIN_5
#define CAN2_EXTERNAL_COMM_RX_GPIO_Port GPIOB
#define CAN2_EXTERNAL_COMM_TX_Pin GPIO_PIN_6
#define CAN2_EXTERNAL_COMM_TX_GPIO_Port GPIOB
#define EXTI_BDI_ENABLE_Pin GPIO_PIN_7
#define EXTI_BDI_ENABLE_GPIO_Port GPIOB
#define EXTI_BDI_ENABLE_EXTI_IRQn EXTI9_5_IRQn
#define CAN1_COMM_RX_Pin GPIO_PIN_8
#define CAN1_COMM_RX_GPIO_Port GPIOB
#define CAN1_COMM_TX_Pin GPIO_PIN_9
#define CAN1_COMM_TX_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
