#include "../../domain/interface/bdi_i2c.h"
#include "../../domain/interface/signal.h"
#include "../../domain/interface/timer.h"
#include "../../lib/drv_lp5018/drv_lp5018.h"
#include "../service/i2c_port.h"
#include "../service/platform.h"

#define self (bdi_i2c_private)

#define BDI_DISPLAY_TICK (10000u)
#define BDI_ENABLE_DEBOUNCE_TICK (30u)

#define BDI_LED_BRIGHTNESS (0xFF)    // (0 ~ 255)
#define STATE_LED_BRIGHTNESS (0x08)  // (0 ~ 255)

static struct {
    void* bdi_display_timer;

    uint8_t last_soc_led_count;
    BmsPackState last_pack_state;

    uint16_t last_soc;

    AppErrorInfo error_info;
} bdi_i2c_private;

// DRV_LP5018_OUT_0 : SOC 10
// DRV_LP5018_OUT_1 : SOC 9
// DRV_LP5018_OUT_2 : X
// DRV_LP5018_OUT_3 : SOC 8
// DRV_LP5018_OUT_4 : SOC 7
// DRV_LP5018_OUT_5 : SOC 6
// DRV_LP5018_OUT_6 : X
// DRV_LP5018_OUT_7 : SOC 5
// DRV_LP5018_OUT_8 : SOC 4
// DRV_LP5018_OUT_9 : SOC 3
// DRV_LP5018_OUT_10 : SOC 2
// DRV_LP5018_OUT_11 : X
// DRV_LP5018_OUT_12 : SOC 1 + RED
// DRV_LP5018_OUT_13 : X
// DRV_LP5018_OUT_14 : GREEN
// DRV_LP5018_OUT_15 : BLUE
// DRV_LP5018_OUT_16 : X
// DRV_LP5018_OUT_17 : X
// DRV_LP5018_OUT_18 : X
// DRV_LP5018_OUT_19 : X
// DRV_LP5018_OUT_20 : X
// DRV_LP5018_OUT_21 : X
// DRV_LP5018_OUT_22 : X
// DRV_LP5018_OUT_23 : X

static const uint8_t soc_outputs[] = {
    DRV_LP5018_OUT_12, DRV_LP5018_OUT_10, DRV_LP5018_OUT_9, DRV_LP5018_OUT_8, DRV_LP5018_OUT_7,
    DRV_LP5018_OUT_5,  DRV_LP5018_OUT_4,  DRV_LP5018_OUT_3, DRV_LP5018_OUT_1, DRV_LP5018_OUT_0,
};

static int _write(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                  const uint16_t length) {
    return I2cPort_Write(device_address << 1, register_address, data, length);
}

static int _read(uint8_t device_address, uint8_t register_address, uint8_t* data,
                 const uint16_t length) {
    return I2cPort_Read(device_address << 1, register_address, data, length);
}

static int _setSocLed(uint16_t soc) {
    if (soc > 1000) {
        return -1;
    }

    uint8_t led_count = sizeof(soc_outputs) / sizeof(soc_outputs[0]);
    uint8_t soc_led_count;

    if (soc <= 50) {
        // 0~5%: all off
        soc_led_count = 0;

    } else if (soc >= 950) {
        // 95~100%: all on
        soc_led_count = led_count;

    } else {
        // round to the nearest integer
        soc_led_count = (soc * led_count + 500) / 1000;
    }

    if (soc_led_count == self.last_soc_led_count) {
        return 0;
    }

    for (uint8_t i = 0; i < led_count; i++) {
        uint8_t brightness = (i < soc_led_count) ? BDI_LED_BRIGHTNESS : 0x00;
        if (DrvLp5018_SetOutput(soc_outputs[i], brightness) != 0) {
            return -1;
        }
    }

    self.last_soc_led_count = soc_led_count;

    return 0;
}

static void _onBdiDisplayEnabled() {
    _setSocLed(self.last_soc);

    Timer_Start(self.bdi_display_timer, BDI_DISPLAY_TICK, true, NULL);

    Signal_DisableBdiEnableOnCallback();
}

static void _onBdiDisplayTimeout(void* context) {
    (void)context;

    _setSocLed(0);

    Signal_RegisterBdiEnableOnCallback(_onBdiDisplayEnabled, BDI_ENABLE_DEBOUNCE_TICK);
}

static int _setStateLed(BmsPackState pack_state) {
    uint8_t rgb[3] = {0};

    switch (pack_state) {
        case BMS_PACK_STATE_IDLE: {  // green
            rgb[0] = 0x00;
            rgb[1] = STATE_LED_BRIGHTNESS;
            rgb[2] = 0x00;
            break;
        }

        case BMS_PACK_STATE_CHARGE_CC:
        case BMS_PACK_STATE_CHARGE_CV:
        case BMS_PACK_STATE_CHARGE: {  // yellow
            rgb[0] = STATE_LED_BRIGHTNESS;
            rgb[1] = STATE_LED_BRIGHTNESS;
            rgb[2] = 0x00;
            break;
        }

        case BMS_PACK_STATE_DISCHARGE: {  // blue
            rgb[0] = 0x00;
            rgb[1] = 0x00;
            rgb[2] = STATE_LED_BRIGHTNESS;
            break;
        }

        case BMS_PACK_STATE_REGENERATE: {  // purple
            rgb[0] = STATE_LED_BRIGHTNESS;
            rgb[1] = 0x00;
            rgb[2] = STATE_LED_BRIGHTNESS;
            break;
        }

        case BMS_PACK_STATE_SLEEP: {  // off
            rgb[0] = 0x00;
            rgb[1] = 0x00;
            rgb[2] = 0x00;
            break;
        }

        case BMS_PACK_STATE_PERMANENT_FAILURE:
        case BMS_PACK_STATE_PROTECTION: {  // red
            rgb[0] = STATE_LED_BRIGHTNESS;
            rgb[1] = 0x00;
            rgb[2] = 0x00;
            break;
        }
    }

    // TODO: check LED
    if (DrvLp5018_SetOutput(DRV_LP5018_OUT_13, rgb[0]) != 0) {
        return -1;
    }

    if (DrvLp5018_SetOutput(DRV_LP5018_OUT_15, rgb[2]) != 0) {
        return -1;
    }

    if (DrvLp5018_SetOutput(DRV_LP5018_OUT_14, rgb[1]) != 0) {
        return -1;
    }

    self.last_pack_state = pack_state;

    return 0;
}

AppResult BdiI2c_Initialize(const BmsConfig* config) {
    (void)config;

    if (!I2cPort_IsInitialized()) {
        if (I2cPort_Initialize() != APP_RESULT_OK) {
            I2cPort_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    if (DrvLp5018_Initialize(&(DrvLp5018Config){.device_address = DRV_LP5018_DEVICE_ADDRESS_0},
                             &(DrvLp5018Interface){
                                 .write = _write,
                                 .read = _read,
                                 .delayMs = (void (*)(uint32_t))osDelay,
                             }) != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BDI_I2C_DRV_INIT);
        return APP_RESULT_FAIL;
    }

    Signal_RegisterBdiEnableOnCallback(_onBdiDisplayEnabled, BDI_ENABLE_DEBOUNCE_TICK);

    if (self.bdi_display_timer == NULL) {
        self.bdi_display_timer = Timer_Create(_onBdiDisplayTimeout, true, NULL);
        if (self.bdi_display_timer == NULL) {
            Timer_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    self.last_soc_led_count = UINT8_MAX;

    return APP_RESULT_OK;
}
AppResult BdiI2c_Deinitialize(void) {
    if (I2cPort_IsInitialized()) {
        if (I2cPort_Deinitialize() != APP_RESULT_OK) {
            I2cPort_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    if (self.bdi_display_timer != NULL) {
        Timer_Dispose(self.bdi_display_timer);
        self.bdi_display_timer = NULL;
    }

    return APP_RESULT_OK;
}

void BdiI2c_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        info->code = APP_ERROR_NOT_IMPLEMENT;
    }
}

AppResult BdiI2c_Update(const BmsData* data) {
    if (data == NULL) {
        return APP_RESULT_INVALID_ARG;
    }

    if (_setStateLed(data->pack_state) != 0) {
        return APP_RESULT_FAIL;
    }

    self.last_soc = data->soc.display_soc;

    return APP_RESULT_OK;
}

void BdiI2c_Test(const BmsData* data) {
    (void)data;

    uint8_t rgb[3] = {0};

    rgb[0] = STATE_LED_BRIGHTNESS;
    rgb[1] = 0x00;
    rgb[2] = 0x00;

    if (DrvLp5018_SetOutput(DRV_LP5018_OUT_13, rgb[0]) != 0) {
        return;
    }

    if (DrvLp5018_SetOutput(DRV_LP5018_OUT_15, rgb[2]) != 0) {
        return;
    }

    if (DrvLp5018_SetOutput(DRV_LP5018_OUT_14, rgb[1]) != 0) {
        return;
    }
}