#ifndef PLATFORM_H

#if defined(_MSC_VER) || defined(MOCK_BUILD) || defined(WIN32_BUILD)

#elif defined(USE_HAL_DRIVER) || defined(STM32F4xx)
#include "../../../Core/Inc/adc.h"
#include "../../../Core/Inc/can.h"
#include "../../../Core/Inc/gpio.h"
#include "../../../Core/Inc/i2c.h"
#include "../../../Core/Inc/main.h"
#include "../../../Core/Inc/rtc.h"
#include "../../../Core/Inc/spi.h"
#include "../../../Core/Inc/tim.h"
#include "../../../Core/Inc/usart.h"
#include "../../../Core/Inc/usb_otg.h"
#include "../../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.h"
#include "../../../Middlewares/Third_Party/FreeRTOS/Source/include/FreeRTOS.h"
#include "../../../Middlewares/Third_Party/FreeRTOS/Source/include/timers.h"

#define I2C_RTC_BDI (&hi2c2)
#define I2C_CURRENT (&hi2c3)

#define SPI_NAND_FLASH (&hspi2)

#define ADC_TEMPERATURE (&hadc1)

#define CAN_UI_COMM (&hcan1)
#define CAN_HGP (&hcan2)

#define UART_HGP (&huart1)
#define UART_BANK_VOLTAGE (&huart2)
#define UART_UI_COMM (&huart4)

#ifndef CCMRAM
#if defined(STM32F407xx)
#define CCMRAM __attribute__((section(".ccmram")))
#else
#define CCMRAM
#endif
#endif

#endif

#endif  // PLATFORM_H

/*
SystemTime (system_time) -> RTC (rtc_port) -> I2C2 (i2c_port)-> MCP7941x (drv_mcp7941x)

NVS -> RetentionStorage (retention_storage) -> RTC (rtc_port) -> I2C2 (i2c_port)
-> MCP7941x (drv_mcp7941x) BDI (bdi_i2c) -> I2C2 (i2c_port) -> LP5018 (drv_lp5018)

I2C2 : RTC , BDI
I2C3 : Current (ADS1115 CH0), Temperature (ADS1115 CH1, CH2, CH3)
*/
