################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/lib/tinyusb/tusb.c 

OBJS += \
./App/lib/tinyusb/tusb.o 

C_DEPS += \
./App/lib/tinyusb/tusb.d 


# Each subdirectory must supply rules for building sources it contributes
App/lib/tinyusb/%.o App/lib/tinyusb/%.su App/lib/tinyusb/%.cyclo: ../App/lib/tinyusb/%.c App/lib/tinyusb/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App -I"../App/lib/tinyusb" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-lib-2f-tinyusb

clean-App-2f-lib-2f-tinyusb:
	-$(RM) ./App/lib/tinyusb/tusb.cyclo ./App/lib/tinyusb/tusb.d ./App/lib/tinyusb/tusb.o ./App/lib/tinyusb/tusb.su

.PHONY: clean-App-2f-lib-2f-tinyusb

