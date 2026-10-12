#include "drv_ads1115.h"

#include "drv_ads1115_protocol.h"

#define self drv_ads1115_private

typedef struct {
    bool is_initialized;
    DrvAds1115Interface interface;
} DrvAds1115Private;

static DrvAds1115Private drv_ads1115_private;

int DrvAds1115_Initialize(DrvAds1115Interface* interface) {
    if (interface == NULL || interface->write == NULL || interface->read == NULL ||
        interface->delayMs == NULL) {
        return -1;
    }

    DrvAds1115_RegisterInterface(interface);

    memcpy(&self.interface, interface, sizeof(DrvAds1115Interface));

    int result = DrvAds1115_Reset();
    if (result != 0) {
        return result;
    }

    result = DrvAds1115_WaitUntilReady();
    if (result != 0) {
        return result;
    }

    self.is_initialized = true;

    return 0;
}

int DrvAds1115_Reset() {
    uint8_t buffer = RESET;

    // general call (address : 0x00) for reset
    int result = self.interface.write(0x00, &buffer, 1);
    if (result != 0) {
        return result;
    }

    self.interface.delayMs(10);

    return 0;
}

bool DrvAds1115_IsBusy() {
    ConfigRegister config;

    (void)DrvAds1115_ReadConfig(&config);

    if (config.os == 0) {
        return true;
    }

    return false;
}

int DrvAds1115_WaitUntilReady() {
    int try_count = 1000;

    while (DrvAds1115_IsBusy()) {
        self.interface.delayMs(1);
        if (--try_count < 0) {
            return -1;
        }
    }

    return 0;
}

int DrvAds1115_StartConversion(const DrvAds1115ChannelMode channel_mode,
                               const DrvAds1115DataSampleRate data_sample_rate,
                               const DrvAds1115Gain gain, bool is_single_shot) {
    if (channel_mode < DRV_ADS1115_CHANNEL_MODE_DIFFERENTIAL_CHANNEL0_1 ||
        channel_mode > DRV_ADS1115_CHANNEL_MODE_SINGLE_ENDED_CHANNEL3 ||
        data_sample_rate < DRV_ADS1115_DATA_SAMPLE_RATE_8_SAMPLES_PER_SECOND ||
        data_sample_rate > DRV_ADS1115_DATA_SAMPLE_RATE_860_SAMPLES_PER_SECOND ||
        gain < DRV_ADS1115_GAIN_V6_144 || gain > DRV_ADS1115_GAIN_V0_256) {
        return -1;
    }

    ConfigRegister config;
    int result = DrvAds1115_ReadConfig(&config);
    if (result != 0) {
        return result;
    }

    config.os = 1;
    config.dr = data_sample_rate;
    config.pga = gain;
    config.mode = is_single_shot ? 1 : 0;
    config.mux = channel_mode;

    return DrvAds1115_WriteConfig(&config);
}

int DrvAds1115_StopConversion() {
    ConfigRegister config;
    int result = DrvAds1115_ReadConfig(&config);
    if (result != 0) {
        return result;
    }

    config.os = 1;
    config.mode = 1;

    return DrvAds1115_WriteConfig(&config);
}

int DrvAds1115_ReadConversion(int16_t* value) {
    if (value == NULL) {
        return -1;
    }

    uint8_t buffer[2] = {0};
    int result = DrvAds1115_ReadRegister(CONVERSION, buffer, sizeof(buffer));
    if (result != 0) {
        return result;
    }

    uint16_t raw_value = (uint16_t)((buffer[0] << 8) | buffer[1]);
    *value = (int16_t)raw_value;

    return 0;
}

float DrvAds1115_ConvertToVoltage(int16_t value, const DrvAds1115Gain gain) {
    switch (gain) {
        case DRV_ADS1115_GAIN_V6_144:
            return (float)value * 6.144f / 32768.0f;
        case DRV_ADS1115_GAIN_V4_096:
            return (float)value * 4.096f / 32768.0f;
        case DRV_ADS1115_GAIN_V2_048:
            return (float)value * 2.048f / 32768.0f;
        case DRV_ADS1115_GAIN_V1_024:
            return (float)value * 1.024f / 32768.0f;
        case DRV_ADS1115_GAIN_V0_512:
            return (float)value * 0.512f / 32768.0f;
        case DRV_ADS1115_GAIN_V0_256:
            return (float)value * 0.256f / 32768.0f;
        default:
            return -1.0f;
    }
}
