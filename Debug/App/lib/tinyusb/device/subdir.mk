################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/lib/tinyusb/device/usbd.c \
../App/lib/tinyusb/device/usbd_control.c 

OBJS += \
./App/lib/tinyusb/device/usbd.o \
./App/lib/tinyusb/device/usbd_control.o 

C_DEPS += \
./App/lib/tinyusb/device/usbd.d \
./App/lib/tinyusb/device/usbd_control.d 


# Each subdirectory must supply rules for building sources it contributes
App/lib/tinyusb/device/%.o App/lib/tinyusb/device/%.su App/lib/tinyusb/device/%.cyclo: ../App/lib/tinyusb/device/%.c App/lib/tinyusb/device/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App -I"../App/lib/tinyusb" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-lib-2f-tinyusb-2f-device

clean-App-2f-lib-2f-tinyusb-2f-device:
	-$(RM) ./App/lib/tinyusb/device/usbd.cyclo ./App/lib/tinyusb/device/usbd.d ./App/lib/tinyusb/device/usbd.o ./App/lib/tinyusb/device/usbd.su ./App/lib/tinyusb/device/usbd_control.cyclo ./App/lib/tinyusb/device/usbd_control.d ./App/lib/tinyusb/device/usbd_control.o ./App/lib/tinyusb/device/usbd_control.su

.PHONY: clean-App-2f-lib-2f-tinyusb-2f-device

