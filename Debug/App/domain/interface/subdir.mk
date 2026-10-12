################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/domain/interface/bank_voltage.c \
../App/domain/interface/bdi_can.c \
../App/domain/interface/bdi_i2c.c \
../App/domain/interface/charger_can.c \
../App/domain/interface/config_storage.c \
../App/domain/interface/current.c \
../App/domain/interface/data_log_storage.c \
../App/domain/interface/error_log_storage.c \
../App/domain/interface/event_log_storage.c \
../App/domain/interface/external_comm.c \
../App/domain/interface/retention_storage.c \
../App/domain/interface/rtos.c \
../App/domain/interface/signal.c \
../App/domain/interface/system.c \
../App/domain/interface/system_time.c \
../App/domain/interface/temperature.c \
../App/domain/interface/timer.c \
../App/domain/interface/ui_comm.c 

OBJS += \
./App/domain/interface/bank_voltage.o \
./App/domain/interface/bdi_can.o \
./App/domain/interface/bdi_i2c.o \
./App/domain/interface/charger_can.o \
./App/domain/interface/config_storage.o \
./App/domain/interface/current.o \
./App/domain/interface/data_log_storage.o \
./App/domain/interface/error_log_storage.o \
./App/domain/interface/event_log_storage.o \
./App/domain/interface/external_comm.o \
./App/domain/interface/retention_storage.o \
./App/domain/interface/rtos.o \
./App/domain/interface/signal.o \
./App/domain/interface/system.o \
./App/domain/interface/system_time.o \
./App/domain/interface/temperature.o \
./App/domain/interface/timer.o \
./App/domain/interface/ui_comm.o 

C_DEPS += \
./App/domain/interface/bank_voltage.d \
./App/domain/interface/bdi_can.d \
./App/domain/interface/bdi_i2c.d \
./App/domain/interface/charger_can.d \
./App/domain/interface/config_storage.d \
./App/domain/interface/current.d \
./App/domain/interface/data_log_storage.d \
./App/domain/interface/error_log_storage.d \
./App/domain/interface/event_log_storage.d \
./App/domain/interface/external_comm.d \
./App/domain/interface/retention_storage.d \
./App/domain/interface/rtos.d \
./App/domain/interface/signal.d \
./App/domain/interface/system.d \
./App/domain/interface/system_time.d \
./App/domain/interface/temperature.d \
./App/domain/interface/timer.d \
./App/domain/interface/ui_comm.d 


# Each subdirectory must supply rules for building sources it contributes
App/domain/interface/%.o App/domain/interface/%.su App/domain/interface/%.cyclo: ../App/domain/interface/%.c App/domain/interface/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App -I"../App/lib/tinyusb" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-domain-2f-interface

clean-App-2f-domain-2f-interface:
	-$(RM) ./App/domain/interface/bank_voltage.cyclo ./App/domain/interface/bank_voltage.d ./App/domain/interface/bank_voltage.o ./App/domain/interface/bank_voltage.su ./App/domain/interface/bdi_can.cyclo ./App/domain/interface/bdi_can.d ./App/domain/interface/bdi_can.o ./App/domain/interface/bdi_can.su ./App/domain/interface/bdi_i2c.cyclo ./App/domain/interface/bdi_i2c.d ./App/domain/interface/bdi_i2c.o ./App/domain/interface/bdi_i2c.su ./App/domain/interface/charger_can.cyclo ./App/domain/interface/charger_can.d ./App/domain/interface/charger_can.o ./App/domain/interface/charger_can.su ./App/domain/interface/config_storage.cyclo ./App/domain/interface/config_storage.d ./App/domain/interface/config_storage.o ./App/domain/interface/config_storage.su ./App/domain/interface/current.cyclo ./App/domain/interface/current.d ./App/domain/interface/current.o ./App/domain/interface/current.su ./App/domain/interface/data_log_storage.cyclo ./App/domain/interface/data_log_storage.d ./App/domain/interface/data_log_storage.o ./App/domain/interface/data_log_storage.su ./App/domain/interface/error_log_storage.cyclo ./App/domain/interface/error_log_storage.d ./App/domain/interface/error_log_storage.o ./App/domain/interface/error_log_storage.su ./App/domain/interface/event_log_storage.cyclo ./App/domain/interface/event_log_storage.d ./App/domain/interface/event_log_storage.o ./App/domain/interface/event_log_storage.su ./App/domain/interface/external_comm.cyclo ./App/domain/interface/external_comm.d ./App/domain/interface/external_comm.o ./App/domain/interface/external_comm.su ./App/domain/interface/retention_storage.cyclo ./App/domain/interface/retention_storage.d ./App/domain/interface/retention_storage.o ./App/domain/interface/retention_storage.su ./App/domain/interface/rtos.cyclo ./App/domain/interface/rtos.d ./App/domain/interface/rtos.o ./App/domain/interface/rtos.su ./App/domain/interface/signal.cyclo ./App/domain/interface/signal.d ./App/domain/interface/signal.o ./App/domain/interface/signal.su ./App/domain/interface/system.cyclo ./App/domain/interface/system.d ./App/domain/interface/system.o ./App/domain/interface/system.su ./App/domain/interface/system_time.cyclo ./App/domain/interface/system_time.d ./App/domain/interface/system_time.o ./App/domain/interface/system_time.su ./App/domain/interface/temperature.cyclo ./App/domain/interface/temperature.d ./App/domain/interface/temperature.o ./App/domain/interface/temperature.su ./App/domain/interface/timer.cyclo ./App/domain/interface/timer.d ./App/domain/interface/timer.o ./App/domain/interface/timer.su ./App/domain/interface/ui_comm.cyclo ./App/domain/interface/ui_comm.d ./App/domain/interface/ui_comm.o ./App/domain/interface/ui_comm.su

.PHONY: clean-App-2f-domain-2f-interface

