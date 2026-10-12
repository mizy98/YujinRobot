################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/application/rtos/rtos_common.c \
../App/application/rtos/task_current.c \
../App/application/rtos/task_external_comm.c \
../App/application/rtos/task_idle.c \
../App/application/rtos/task_protection.c \
../App/application/rtos/task_rtc.c \
../App/application/rtos/task_storage.c \
../App/application/rtos/task_temperature.c \
../App/application/rtos/task_ui_comm.c \
../App/application/rtos/task_ui_comm_usb.c \
../App/application/rtos/task_voltage.c 

OBJS += \
./App/application/rtos/rtos_common.o \
./App/application/rtos/task_current.o \
./App/application/rtos/task_external_comm.o \
./App/application/rtos/task_idle.o \
./App/application/rtos/task_protection.o \
./App/application/rtos/task_rtc.o \
./App/application/rtos/task_storage.o \
./App/application/rtos/task_temperature.o \
./App/application/rtos/task_ui_comm.o \
./App/application/rtos/task_ui_comm_usb.o \
./App/application/rtos/task_voltage.o 

C_DEPS += \
./App/application/rtos/rtos_common.d \
./App/application/rtos/task_current.d \
./App/application/rtos/task_external_comm.d \
./App/application/rtos/task_idle.d \
./App/application/rtos/task_protection.d \
./App/application/rtos/task_rtc.d \
./App/application/rtos/task_storage.d \
./App/application/rtos/task_temperature.d \
./App/application/rtos/task_ui_comm.d \
./App/application/rtos/task_ui_comm_usb.d \
./App/application/rtos/task_voltage.d 


# Each subdirectory must supply rules for building sources it contributes
App/application/rtos/%.o App/application/rtos/%.su App/application/rtos/%.cyclo: ../App/application/rtos/%.c App/application/rtos/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App -I"../App/lib/tinyusb" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-application-2f-rtos

clean-App-2f-application-2f-rtos:
	-$(RM) ./App/application/rtos/rtos_common.cyclo ./App/application/rtos/rtos_common.d ./App/application/rtos/rtos_common.o ./App/application/rtos/rtos_common.su ./App/application/rtos/task_current.cyclo ./App/application/rtos/task_current.d ./App/application/rtos/task_current.o ./App/application/rtos/task_current.su ./App/application/rtos/task_external_comm.cyclo ./App/application/rtos/task_external_comm.d ./App/application/rtos/task_external_comm.o ./App/application/rtos/task_external_comm.su ./App/application/rtos/task_idle.cyclo ./App/application/rtos/task_idle.d ./App/application/rtos/task_idle.o ./App/application/rtos/task_idle.su ./App/application/rtos/task_protection.cyclo ./App/application/rtos/task_protection.d ./App/application/rtos/task_protection.o ./App/application/rtos/task_protection.su ./App/application/rtos/task_rtc.cyclo ./App/application/rtos/task_rtc.d ./App/application/rtos/task_rtc.o ./App/application/rtos/task_rtc.su ./App/application/rtos/task_storage.cyclo ./App/application/rtos/task_storage.d ./App/application/rtos/task_storage.o ./App/application/rtos/task_storage.su ./App/application/rtos/task_temperature.cyclo ./App/application/rtos/task_temperature.d ./App/application/rtos/task_temperature.o ./App/application/rtos/task_temperature.su ./App/application/rtos/task_ui_comm.cyclo ./App/application/rtos/task_ui_comm.d ./App/application/rtos/task_ui_comm.o ./App/application/rtos/task_ui_comm.su ./App/application/rtos/task_ui_comm_usb.cyclo ./App/application/rtos/task_ui_comm_usb.d ./App/application/rtos/task_ui_comm_usb.o ./App/application/rtos/task_ui_comm_usb.su ./App/application/rtos/task_voltage.cyclo ./App/application/rtos/task_voltage.d ./App/application/rtos/task_voltage.o ./App/application/rtos/task_voltage.su

.PHONY: clean-App-2f-application-2f-rtos

