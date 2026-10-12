################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/lib/tinyusb/portable/chipidea/ci_hs/dcd_ci_hs.c \
../App/lib/tinyusb/portable/chipidea/ci_hs/hcd_ci_hs.c 

OBJS += \
./App/lib/tinyusb/portable/chipidea/ci_hs/dcd_ci_hs.o \
./App/lib/tinyusb/portable/chipidea/ci_hs/hcd_ci_hs.o 

C_DEPS += \
./App/lib/tinyusb/portable/chipidea/ci_hs/dcd_ci_hs.d \
./App/lib/tinyusb/portable/chipidea/ci_hs/hcd_ci_hs.d 


# Each subdirectory must supply rules for building sources it contributes
App/lib/tinyusb/portable/chipidea/ci_hs/%.o App/lib/tinyusb/portable/chipidea/ci_hs/%.su App/lib/tinyusb/portable/chipidea/ci_hs/%.cyclo: ../App/lib/tinyusb/portable/chipidea/ci_hs/%.c App/lib/tinyusb/portable/chipidea/ci_hs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App -I"../App/lib/tinyusb" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-lib-2f-tinyusb-2f-portable-2f-chipidea-2f-ci_hs

clean-App-2f-lib-2f-tinyusb-2f-portable-2f-chipidea-2f-ci_hs:
	-$(RM) ./App/lib/tinyusb/portable/chipidea/ci_hs/dcd_ci_hs.cyclo ./App/lib/tinyusb/portable/chipidea/ci_hs/dcd_ci_hs.d ./App/lib/tinyusb/portable/chipidea/ci_hs/dcd_ci_hs.o ./App/lib/tinyusb/portable/chipidea/ci_hs/dcd_ci_hs.su ./App/lib/tinyusb/portable/chipidea/ci_hs/hcd_ci_hs.cyclo ./App/lib/tinyusb/portable/chipidea/ci_hs/hcd_ci_hs.d ./App/lib/tinyusb/portable/chipidea/ci_hs/hcd_ci_hs.o ./App/lib/tinyusb/portable/chipidea/ci_hs/hcd_ci_hs.su

.PHONY: clean-App-2f-lib-2f-tinyusb-2f-portable-2f-chipidea-2f-ci_hs

