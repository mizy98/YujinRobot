################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/lib/drv_mt29fx/drv_mt29fx.c \
../App/lib/drv_mt29fx/drv_mt29fx_protocol.c 

OBJS += \
./App/lib/drv_mt29fx/drv_mt29fx.o \
./App/lib/drv_mt29fx/drv_mt29fx_protocol.o 

C_DEPS += \
./App/lib/drv_mt29fx/drv_mt29fx.d \
./App/lib/drv_mt29fx/drv_mt29fx_protocol.d 


# Each subdirectory must supply rules for building sources it contributes
App/lib/drv_mt29fx/%.o App/lib/drv_mt29fx/%.su App/lib/drv_mt29fx/%.cyclo: ../App/lib/drv_mt29fx/%.c App/lib/drv_mt29fx/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App -I"../App/lib/tinyusb" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-lib-2f-drv_mt29fx

clean-App-2f-lib-2f-drv_mt29fx:
	-$(RM) ./App/lib/drv_mt29fx/drv_mt29fx.cyclo ./App/lib/drv_mt29fx/drv_mt29fx.d ./App/lib/drv_mt29fx/drv_mt29fx.o ./App/lib/drv_mt29fx/drv_mt29fx.su ./App/lib/drv_mt29fx/drv_mt29fx_protocol.cyclo ./App/lib/drv_mt29fx/drv_mt29fx_protocol.d ./App/lib/drv_mt29fx/drv_mt29fx_protocol.o ./App/lib/drv_mt29fx/drv_mt29fx_protocol.su

.PHONY: clean-App-2f-lib-2f-drv_mt29fx

