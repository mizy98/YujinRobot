#include "drv_lp5018.h"

#include "drv_lp5018_protocol.h"

DrvLp5018Private drv_lp5018_private;

int DrvLp5018_Initialize(DrvLp5018Config* config, DrvLp5018Interface* interface) {
    if (config == NULL || interface == NULL || interface->write == NULL ||
        interface->read == NULL || interface->delayMs == NULL) {
        return -1;
    }

    if (config->device_address < DRV_LP5018_DEVICE_ADDRESS_0 ||
        config->device_address > DRV_LP5018_DEVICE_ADDRESS_3) {
        return -1;
    }

    self.interface = *interface;
    self.config = *config;

    self.device_address = LP5018_BASE_DEVICE_ADDRESS | (self.config.device_address & 0x03);

    if (DrvLp5018_Reset() != 0) {
        return -1;
    }

    DrvLp5018Config0 config0 = {
        .chip_en = 1,
    };
    if (DrvLp5018_WriteRegister(DEVICE_CONFIG0, &config0.value, sizeof(config0)) != 0) {
        return -1;
    }

    DrvLp5018Config1 config1 = {
        .led_global_off = 0,
        .max_current_option = 1,
        .pwm_dithering = 1,
        .auto_incr_en = 1,
        .power_save_en = 1,
        .log_scale_en = 1,
    };
    if (DrvLp5018_WriteRegister(DEVICE_CONFIG1, &config1.value, sizeof(config1)) != 0) {
        return -1;
    }

    return 0;
}

int DrvLp5018_Reset(void) {
    uint8_t data = 0xFF;
    if (DrvLp5018_WriteRegister(RESET, &data, 1) != 0) {
        return -1;
    }

    return 0;
}

int DrvLp5018_SetOutput(DrvLp5018Output output, uint8_t brightness) {
    if (output < DRV_LP5018_OUT_0 || output > DRV_LP5018_OUT_23) {
        return -1;
    }

    uint8_t address = OUT0_COLOR + output;
    uint8_t brightness_data = brightness;

    return DrvLp5018_WriteRegister(address, &brightness_data, 1);
}

int DrvLp5018_SetOutputs(DrvLp5018Output base_output, uint8_t count, uint8_t brightness) {
    if (base_output < DRV_LP5018_OUT_0 || base_output > DRV_LP5018_OUT_23) {
        return -1;
    }

    if (count == 0 || (base_output + count - 1) > DRV_LP5018_OUT_23) {
        return -1;
    }

    uint8_t address = OUT0_COLOR + base_output;
    uint8_t brightness_data[count];
    for (uint8_t i = 0; i < count; i++) {
        brightness_data[i] = brightness;
    }

    return DrvLp5018_WriteRegister(address, brightness_data, count);
}

int DrvLp5018_SetLed(DrvLp5018Led led, uint8_t brightness, uint8_t red, uint8_t green,
                     uint8_t blue) {
    if (led < DRV_LP5018_LED_0 || led > DRV_LP5018_LED_7) {
        return -1;
    }

    uint8_t led_address = LED0_BRIGHTNESS + led;
    uint8_t led_brightness_data = brightness;

    uint8_t out_address = OUT0_COLOR + (led * 3);
    uint8_t out_color_data[3] = {red, green, blue};

    if (DrvLp5018_WriteRegister(led_address, &led_brightness_data, 1) != 0) {
        return -1;
    }

    if (DrvLp5018_WriteRegister(out_address, out_color_data, sizeof(out_color_data)) != 0) {
        return -1;
    }

    return 0;
}
