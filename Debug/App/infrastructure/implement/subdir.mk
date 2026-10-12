################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/infrastructure/implement/bank_voltage_impl.c \
../App/infrastructure/implement/bdi_i2c_impl.c \
../App/infrastructure/implement/config_storage_impl.c \
../App/infrastructure/implement/current_impl.c \
../App/infrastructure/implement/data_log_storage_impl.c \
../App/infrastructure/implement/error_log_storage_impl.c \
../App/infrastructure/implement/event_log_storage_impl.c \
../App/infrastructure/implement/external_comm_impl.c \
../App/infrastructure/implement/retention_storage_impl.c \
../App/infrastructure/implement/rtos_impl.c \
../App/infrastructure/implement/signal_impl.c \
../App/infrastructure/implement/system_impl.c \
../App/infrastructure/implement/system_time_impl.c \
../App/infrastructure/implement/temperature_impl.c \
../App/infrastructure/implement/timer_impl.c \
../App/infrastructure/implement/ui_comm_impl.c 

OBJS += \
./App/infrastructure/implement/bank_voltage_impl.o \
./App/infrastructure/implement/bdi_i2c_impl.o \
./App/infrastructure/implement/config_storage_impl.o \
./App/infrastructure/implement/current_impl.o \
./App/infrastructure/implement/data_log_storage_impl.o \
./App/infrastructure/implement/error_log_storage_impl.o \
./App/infrastructure/implement/event_log_storage_impl.o \
./App/infrastructure/implement/external_comm_impl.o \
./App/infrastructure/implement/retention_storage_impl.o \
./App/infrastructure/implement/rtos_impl.o \
./App/infrastructure/implement/signal_impl.o \
./App/infrastructure/implement/system_impl.o \
./App/infrastructure/implement/system_time_impl.o \
./App/infrastructure/implement/temperature_impl.o \
./App/infrastructure/implement/timer_impl.o \
./App/infrastructure/implement/ui_comm_impl.o 

C_DEPS += \
./App/infrastructure/implement/bank_voltage_impl.d \
./App/infrastructure/implement/bdi_i2c_impl.d \
./App/infrastructure/implement/config_storage_impl.d \
./App/infrastructure/implement/current_impl.d \
./App/infrastructure/implement/data_log_storage_impl.d \
./App/infrastructure/implement/error_log_storage_impl.d \
./App/infrastructure/implement/event_log_storage_impl.d \
./App/infrastructure/implement/external_comm_impl.d \
./App/infrastructure/implement/retention_storage_impl.d \
./App/infrastructure/implement/rtos_impl.d \
./App/infrastructure/implement/signal_impl.d \
./App/infrastructure/implement/system_impl.d \
./App/infrastructure/implement/system_time_impl.d \
./App/infrastructure/implement/temperature_impl.d \
./App/infrastructure/implement/timer_impl.d \
./App/infrastructure/implement/ui_comm_impl.d 


# Each subdirectory must supply rules for building sources it contributes
App/infrastructure/implement/%.o App/infrastructure/implement/%.su App/infrastructure/implement/%.cyclo: ../App/infrastructure/implement/%.c App/infrastructure/implement/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App -I"../App/lib/tinyusb" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-infrastructure-2f-implement

clean-App-2f-infrastructure-2f-implement:
	-$(RM) ./App/infrastructure/implement/bank_voltage_impl.cyclo ./App/infrastructure/implement/bank_voltage_impl.d ./App/infrastructure/implement/bank_voltage_impl.o ./App/infrastructure/implement/bank_voltage_impl.su ./App/infrastructure/implement/bdi_i2c_impl.cyclo ./App/infrastructure/implement/bdi_i2c_impl.d ./App/infrastructure/implement/bdi_i2c_impl.o ./App/infrastructure/implement/bdi_i2c_impl.su ./App/infrastructure/implement/config_storage_impl.cyclo ./App/infrastructure/implement/config_storage_impl.d ./App/infrastructure/implement/config_storage_impl.o ./App/infrastructure/implement/config_storage_impl.su ./App/infrastructure/implement/current_impl.cyclo ./App/infrastructure/implement/current_impl.d ./App/infrastructure/implement/current_impl.o ./App/infrastructure/implement/current_impl.su ./App/infrastructure/implement/data_log_storage_impl.cyclo ./App/infrastructure/implement/data_log_storage_impl.d ./App/infrastructure/implement/data_log_storage_impl.o ./App/infrastructure/implement/data_log_storage_impl.su ./App/infrastructure/implement/error_log_storage_impl.cyclo ./App/infrastructure/implement/error_log_storage_impl.d ./App/infrastructure/implement/error_log_storage_impl.o ./App/infrastructure/implement/error_log_storage_impl.su ./App/infrastructure/implement/event_log_storage_impl.cyclo ./App/infrastructure/implement/event_log_storage_impl.d ./App/infrastructure/implement/event_log_storage_impl.o ./App/infrastructure/implement/event_log_storage_impl.su ./App/infrastructure/implement/external_comm_impl.cyclo ./App/infrastructure/implement/external_comm_impl.d ./App/infrastructure/implement/external_comm_impl.o ./App/infrastructure/implement/external_comm_impl.su ./App/infrastructure/implement/retention_storage_impl.cyclo ./App/infrastructure/implement/retention_storage_impl.d ./App/infrastructure/implement/retention_storage_impl.o ./App/infrastructure/implement/retention_storage_impl.su ./App/infrastructure/implement/rtos_impl.cyclo ./App/infrastructure/implement/rtos_impl.d ./App/infrastructure/implement/rtos_impl.o ./App/infrastructure/implement/rtos_impl.su ./App/infrastructure/implement/signal_impl.cyclo ./App/infrastructure/implement/signal_impl.d ./App/infrastructure/implement/signal_impl.o ./App/infrastructure/implement/signal_impl.su ./App/infrastructure/implement/system_impl.cyclo ./App/infrastructure/implement/system_impl.d ./App/infrastructure/implement/system_impl.o ./App/infrastructure/implement/system_impl.su ./App/infrastructure/implement/system_time_impl.cyclo ./App/infrastructure/implement/system_time_impl.d ./App/infrastructure/implement/system_time_impl.o ./App/infrastructure/implement/system_time_impl.su ./App/infrastructure/implement/temperature_impl.cyclo ./App/infrastructure/implement/temperature_impl.d ./App/infrastructure/implement/temperature_impl.o ./App/infrastructure/implement/temperature_impl.su ./App/infrastructure/implement/timer_impl.cyclo ./App/infrastructure/implement/timer_impl.d ./App/infrastructure/implement/timer_impl.o ./App/infrastructure/implement/timer_impl.su ./App/infrastructure/implement/ui_comm_impl.cyclo ./App/infrastructure/implement/ui_comm_impl.d ./App/infrastructure/implement/ui_comm_impl.o ./App/infrastructure/implement/ui_comm_impl.su

.PHONY: clean-App-2f-infrastructure-2f-implement

