################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/domain/service/bdi.c \
../App/domain/service/black_box.c \
../App/domain/service/buzzer.c \
../App/domain/service/charger.c \
../App/domain/service/cluster.c \
../App/domain/service/configurator.c \
../App/domain/service/contactor.c \
../App/domain/service/crc16_modbus.c \
../App/domain/service/filter.c \
../App/domain/service/indicator.c \
../App/domain/service/ntc_thermistor.c \
../App/domain/service/nvs.c \
../App/domain/service/pack_state.c \
../App/domain/service/protection.c \
../App/domain/service/rescue.c \
../App/domain/service/soc.c \
../App/domain/service/soc_lfp.c \
../App/domain/service/soc_ncm.c \
../App/domain/service/test_soh.c \
../App/domain/service/thermostat.c \
../App/domain/service/threshold_control.c \
../App/domain/service/ui_protocol.c \
../App/domain/service/validator.c 

OBJS += \
./App/domain/service/bdi.o \
./App/domain/service/black_box.o \
./App/domain/service/buzzer.o \
./App/domain/service/charger.o \
./App/domain/service/cluster.o \
./App/domain/service/configurator.o \
./App/domain/service/contactor.o \
./App/domain/service/crc16_modbus.o \
./App/domain/service/filter.o \
./App/domain/service/indicator.o \
./App/domain/service/ntc_thermistor.o \
./App/domain/service/nvs.o \
./App/domain/service/pack_state.o \
./App/domain/service/protection.o \
./App/domain/service/rescue.o \
./App/domain/service/soc.o \
./App/domain/service/soc_lfp.o \
./App/domain/service/soc_ncm.o \
./App/domain/service/test_soh.o \
./App/domain/service/thermostat.o \
./App/domain/service/threshold_control.o \
./App/domain/service/ui_protocol.o \
./App/domain/service/validator.o 

C_DEPS += \
./App/domain/service/bdi.d \
./App/domain/service/black_box.d \
./App/domain/service/buzzer.d \
./App/domain/service/charger.d \
./App/domain/service/cluster.d \
./App/domain/service/configurator.d \
./App/domain/service/contactor.d \
./App/domain/service/crc16_modbus.d \
./App/domain/service/filter.d \
./App/domain/service/indicator.d \
./App/domain/service/ntc_thermistor.d \
./App/domain/service/nvs.d \
./App/domain/service/pack_state.d \
./App/domain/service/protection.d \
./App/domain/service/rescue.d \
./App/domain/service/soc.d \
./App/domain/service/soc_lfp.d \
./App/domain/service/soc_ncm.d \
./App/domain/service/test_soh.d \
./App/domain/service/thermostat.d \
./App/domain/service/threshold_control.d \
./App/domain/service/ui_protocol.d \
./App/domain/service/validator.d 


# Each subdirectory must supply rules for building sources it contributes
App/domain/service/%.o App/domain/service/%.su App/domain/service/%.cyclo: ../App/domain/service/%.c App/domain/service/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App -I"../App/lib/tinyusb" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-domain-2f-service

clean-App-2f-domain-2f-service:
	-$(RM) ./App/domain/service/bdi.cyclo ./App/domain/service/bdi.d ./App/domain/service/bdi.o ./App/domain/service/bdi.su ./App/domain/service/black_box.cyclo ./App/domain/service/black_box.d ./App/domain/service/black_box.o ./App/domain/service/black_box.su ./App/domain/service/buzzer.cyclo ./App/domain/service/buzzer.d ./App/domain/service/buzzer.o ./App/domain/service/buzzer.su ./App/domain/service/charger.cyclo ./App/domain/service/charger.d ./App/domain/service/charger.o ./App/domain/service/charger.su ./App/domain/service/cluster.cyclo ./App/domain/service/cluster.d ./App/domain/service/cluster.o ./App/domain/service/cluster.su ./App/domain/service/configurator.cyclo ./App/domain/service/configurator.d ./App/domain/service/configurator.o ./App/domain/service/configurator.su ./App/domain/service/contactor.cyclo ./App/domain/service/contactor.d ./App/domain/service/contactor.o ./App/domain/service/contactor.su ./App/domain/service/crc16_modbus.cyclo ./App/domain/service/crc16_modbus.d ./App/domain/service/crc16_modbus.o ./App/domain/service/crc16_modbus.su ./App/domain/service/filter.cyclo ./App/domain/service/filter.d ./App/domain/service/filter.o ./App/domain/service/filter.su ./App/domain/service/indicator.cyclo ./App/domain/service/indicator.d ./App/domain/service/indicator.o ./App/domain/service/indicator.su ./App/domain/service/ntc_thermistor.cyclo ./App/domain/service/ntc_thermistor.d ./App/domain/service/ntc_thermistor.o ./App/domain/service/ntc_thermistor.su ./App/domain/service/nvs.cyclo ./App/domain/service/nvs.d ./App/domain/service/nvs.o ./App/domain/service/nvs.su ./App/domain/service/pack_state.cyclo ./App/domain/service/pack_state.d ./App/domain/service/pack_state.o ./App/domain/service/pack_state.su ./App/domain/service/protection.cyclo ./App/domain/service/protection.d ./App/domain/service/protection.o ./App/domain/service/protection.su ./App/domain/service/rescue.cyclo ./App/domain/service/rescue.d ./App/domain/service/rescue.o ./App/domain/service/rescue.su ./App/domain/service/soc.cyclo ./App/domain/service/soc.d ./App/domain/service/soc.o ./App/domain/service/soc.su ./App/domain/service/soc_lfp.cyclo ./App/domain/service/soc_lfp.d ./App/domain/service/soc_lfp.o ./App/domain/service/soc_lfp.su ./App/domain/service/soc_ncm.cyclo ./App/domain/service/soc_ncm.d ./App/domain/service/soc_ncm.o ./App/domain/service/soc_ncm.su ./App/domain/service/test_soh.cyclo ./App/domain/service/test_soh.d ./App/domain/service/test_soh.o ./App/domain/service/test_soh.su ./App/domain/service/thermostat.cyclo ./App/domain/service/thermostat.d ./App/domain/service/thermostat.o ./App/domain/service/thermostat.su ./App/domain/service/threshold_control.cyclo ./App/domain/service/threshold_control.d ./App/domain/service/threshold_control.o ./App/domain/service/threshold_control.su ./App/domain/service/ui_protocol.cyclo ./App/domain/service/ui_protocol.d ./App/domain/service/ui_protocol.o ./App/domain/service/ui_protocol.su ./App/domain/service/validator.cyclo ./App/domain/service/validator.d ./App/domain/service/validator.o ./App/domain/service/validator.su

.PHONY: clean-App-2f-domain-2f-service

