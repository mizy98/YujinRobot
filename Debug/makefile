################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include App/lib/tinyusb/typec/subdir.mk
-include App/lib/tinyusb/portable/wch/subdir.mk
-include App/lib/tinyusb/portable/valentyusb/eptri/subdir.mk
-include App/lib/tinyusb/portable/ti/msp430x5xx/subdir.mk
-include App/lib/tinyusb/portable/template/subdir.mk
-include App/lib/tinyusb/portable/synopsys/dwc2/subdir.mk
-include App/lib/tinyusb/portable/sunxi/subdir.mk
-include App/lib/tinyusb/portable/st/typec/subdir.mk
-include App/lib/tinyusb/portable/st/stm32_fsdev/subdir.mk
-include App/lib/tinyusb/portable/sony/cxd56/subdir.mk
-include App/lib/tinyusb/portable/renesas/rusb2/subdir.mk
-include App/lib/tinyusb/portable/raspberrypi/rp2040/subdir.mk
-include App/lib/tinyusb/portable/raspberrypi/pio_usb/subdir.mk
-include App/lib/tinyusb/portable/ohci/subdir.mk
-include App/lib/tinyusb/portable/nxp/lpc_ip3516/subdir.mk
-include App/lib/tinyusb/portable/nxp/lpc_ip3511/subdir.mk
-include App/lib/tinyusb/portable/nxp/lpc17_40/subdir.mk
-include App/lib/tinyusb/portable/nxp/khci/subdir.mk
-include App/lib/tinyusb/portable/nuvoton/nuc505/subdir.mk
-include App/lib/tinyusb/portable/nuvoton/nuc121/subdir.mk
-include App/lib/tinyusb/portable/nuvoton/nuc120/subdir.mk
-include App/lib/tinyusb/portable/nordic/nrf5x/subdir.mk
-include App/lib/tinyusb/portable/mindmotion/mm32/subdir.mk
-include App/lib/tinyusb/portable/microchip/samx7x/subdir.mk
-include App/lib/tinyusb/portable/microchip/samg/subdir.mk
-include App/lib/tinyusb/portable/microchip/samd/subdir.mk
-include App/lib/tinyusb/portable/microchip/pic32mz/subdir.mk
-include App/lib/tinyusb/portable/microchip/pic/subdir.mk
-include App/lib/tinyusb/portable/mentor/musb/subdir.mk
-include App/lib/tinyusb/portable/ehci/subdir.mk
-include App/lib/tinyusb/portable/dialog/da146xx/subdir.mk
-include App/lib/tinyusb/portable/chipidea/ci_hs/subdir.mk
-include App/lib/tinyusb/portable/chipidea/ci_fs/subdir.mk
-include App/lib/tinyusb/portable/bridgetek/ft9xx/subdir.mk
-include App/lib/tinyusb/portable/analog/max3421/subdir.mk
-include App/lib/tinyusb/host/subdir.mk
-include App/lib/tinyusb/device/subdir.mk
-include App/lib/tinyusb/common/subdir.mk
-include App/lib/tinyusb/class/video/subdir.mk
-include App/lib/tinyusb/class/vendor/subdir.mk
-include App/lib/tinyusb/class/usbtmc/subdir.mk
-include App/lib/tinyusb/class/printer/subdir.mk
-include App/lib/tinyusb/class/net/subdir.mk
-include App/lib/tinyusb/class/mtp/subdir.mk
-include App/lib/tinyusb/class/msc/subdir.mk
-include App/lib/tinyusb/class/midi/subdir.mk
-include App/lib/tinyusb/class/hid/subdir.mk
-include App/lib/tinyusb/class/dfu/subdir.mk
-include App/lib/tinyusb/class/cdc/subdir.mk
-include App/lib/tinyusb/class/bth/subdir.mk
-include App/lib/tinyusb/class/audio/subdir.mk
-include App/lib/tinyusb/subdir.mk
-include App/lib/drv_mt29fx/subdir.mk
-include App/lib/drv_mcp7941x/subdir.mk
-include App/lib/drv_lp5018/subdir.mk
-include App/lib/drv_bq79616/subdir.mk
-include App/lib/drv_ads1115/subdir.mk
-include App/lib/comm_hgp/subdir.mk
-include App/lib/comm_global_tech/subdir.mk
-include App/infrastructure/service/subdir.mk
-include App/infrastructure/implement/subdir.mk
-include App/domain/service/subdir.mk
-include App/domain/interface/subdir.mk
-include App/application/service/subdir.mk
-include App/application/rtos/subdir.mk
-include App/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := bms_f1
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
bms_f1.elf \

MAP_FILES += \
bms_f1.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
bms_f1.list \


# All Target
all: main-build

# Main-build Target
main-build: bms_f1.elf secondary-outputs

# Tool invocations
bms_f1.elf bms_f1.map: $(OBJS) $(USER_OBJS) C:\01_SMI\study\BMS\YujinRobot\STM32F407VETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "bms_f1.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\01_SMI\study\BMS\YujinRobot\STM32F407VETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="bms_f1.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

bms_f1.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "bms_f1.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) bms_f1.elf bms_f1.list bms_f1.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
