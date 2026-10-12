################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/application/service/rtos_startup.c \
../App/application/service/rtos_status.c \
../App/application/service/startup.c \
../App/application/service/ui_comm_handler.c 

OBJS += \
./App/application/service/rtos_startup.o \
./App/application/service/rtos_status.o \
./App/application/service/startup.o \
./App/application/service/ui_comm_handler.o 

C_DEPS += \
./App/application/service/rtos_startup.d \
./App/application/service/rtos_status.d \
./App/application/service/startup.d \
./App/application/service/ui_comm_handler.d 


# Each subdirectory must supply rules for building sources it contributes
App/application/service/%.o App/application/service/%.su App/application/service/%.cyclo: ../App/application/service/%.c App/application/service/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App -I"../App/lib/tinyusb" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-application-2f-service

clean-App-2f-application-2f-service:
	-$(RM) ./App/application/service/rtos_startup.cyclo ./App/application/service/rtos_startup.d ./App/application/service/rtos_startup.o ./App/application/service/rtos_startup.su ./App/application/service/rtos_status.cyclo ./App/application/service/rtos_status.d ./App/application/service/rtos_status.o ./App/application/service/rtos_status.su ./App/application/service/startup.cyclo ./App/application/service/startup.d ./App/application/service/startup.o ./App/application/service/startup.su ./App/application/service/ui_comm_handler.cyclo ./App/application/service/ui_comm_handler.d ./App/application/service/ui_comm_handler.o ./App/application/service/ui_comm_handler.su

.PHONY: clean-App-2f-application-2f-service

