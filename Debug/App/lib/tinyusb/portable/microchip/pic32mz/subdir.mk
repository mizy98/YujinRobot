################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/lib/tinyusb/portable/microchip/pic32mz/dcd_pic32mz.c 

OBJS += \
./App/lib/tinyusb/portable/microchip/pic32mz/dcd_pic32mz.o 

C_DEPS += \
./App/lib/tinyusb/portable/microchip/pic32mz/dcd_pic32mz.d 


# Each subdirectory must supply rules for building sources it contributes
App/lib/tinyusb/portable/microchip/pic32mz/%.o App/lib/tinyusb/portable/microchip/pic32mz/%.su App/lib/tinyusb/portable/microchip/pic32mz/%.cyclo: ../App/lib/tinyusb/portable/microchip/pic32mz/%.c App/lib/tinyusb/portable/microchip/pic32mz/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App -I"../App/lib/tinyusb" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-lib-2f-tinyusb-2f-portable-2f-microchip-2f-pic32mz

clean-App-2f-lib-2f-tinyusb-2f-portable-2f-microchip-2f-pic32mz:
	-$(RM) ./App/lib/tinyusb/portable/microchip/pic32mz/dcd_pic32mz.cyclo ./App/lib/tinyusb/portable/microchip/pic32mz/dcd_pic32mz.d ./App/lib/tinyusb/portable/microchip/pic32mz/dcd_pic32mz.o ./App/lib/tinyusb/portable/microchip/pic32mz/dcd_pic32mz.su

.PHONY: clean-App-2f-lib-2f-tinyusb-2f-portable-2f-microchip-2f-pic32mz

