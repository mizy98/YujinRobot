################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/infrastructure/service/adc_driver.c \
../App/infrastructure/service/i2c_port.c \
../App/infrastructure/service/nand_flash.c \
../App/infrastructure/service/platform.c \
../App/infrastructure/service/rtc_port.c \
../App/infrastructure/service/tusb_adaptor.c \
../App/infrastructure/service/ui_comm_uart.c \
../App/infrastructure/service/ui_comm_usb.c 

OBJS += \
./App/infrastructure/service/adc_driver.o \
./App/infrastructure/service/i2c_port.o \
./App/infrastructure/service/nand_flash.o \
./App/infrastructure/service/platform.o \
./App/infrastructure/service/rtc_port.o \
./App/infrastructure/service/tusb_adaptor.o \
./App/infrastructure/service/ui_comm_uart.o \
./App/infrastructure/service/ui_comm_usb.o 

C_DEPS += \
./App/infrastructure/service/adc_driver.d \
./App/infrastructure/service/i2c_port.d \
./App/infrastructure/service/nand_flash.d \
./App/infrastructure/service/platform.d \
./App/infrastructure/service/rtc_port.d \
./App/infrastructure/service/tusb_adaptor.d \
./App/infrastructure/service/ui_comm_uart.d \
./App/infrastructure/service/ui_comm_usb.d 


# Each subdirectory must supply rules for building sources it contributes
App/infrastructure/service/%.o App/infrastructure/service/%.su App/infrastructure/service/%.cyclo: ../App/infrastructure/service/%.c App/infrastructure/service/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App -I"../App/lib/tinyusb" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-infrastructure-2f-service

clean-App-2f-infrastructure-2f-service:
	-$(RM) ./App/infrastructure/service/adc_driver.cyclo ./App/infrastructure/service/adc_driver.d ./App/infrastructure/service/adc_driver.o ./App/infrastructure/service/adc_driver.su ./App/infrastructure/service/i2c_port.cyclo ./App/infrastructure/service/i2c_port.d ./App/infrastructure/service/i2c_port.o ./App/infrastructure/service/i2c_port.su ./App/infrastructure/service/nand_flash.cyclo ./App/infrastructure/service/nand_flash.d ./App/infrastructure/service/nand_flash.o ./App/infrastructure/service/nand_flash.su ./App/infrastructure/service/platform.cyclo ./App/infrastructure/service/platform.d ./App/infrastructure/service/platform.o ./App/infrastructure/service/platform.su ./App/infrastructure/service/rtc_port.cyclo ./App/infrastructure/service/rtc_port.d ./App/infrastructure/service/rtc_port.o ./App/infrastructure/service/rtc_port.su ./App/infrastructure/service/tusb_adaptor.cyclo ./App/infrastructure/service/tusb_adaptor.d ./App/infrastructure/service/tusb_adaptor.o ./App/infrastructure/service/tusb_adaptor.su ./App/infrastructure/service/ui_comm_uart.cyclo ./App/infrastructure/service/ui_comm_uart.d ./App/infrastructure/service/ui_comm_uart.o ./App/infrastructure/service/ui_comm_uart.su ./App/infrastructure/service/ui_comm_usb.cyclo ./App/infrastructure/service/ui_comm_usb.d ./App/infrastructure/service/ui_comm_usb.o ./App/infrastructure/service/ui_comm_usb.su

.PHONY: clean-App-2f-infrastructure-2f-service

