#ifndef DRV_ADS1115_H
#define DRV_ADS1115_H

#include "drv_ads1115_define.h"

int DrvAds1115_Initialize(DrvAds1115Interface* interface);
int DrvAds1115_Reset();
bool DrvAds1115_IsBusy();
int DrvAds1115_WaitUntilReady();
int DrvAds1115_StartConversion(const DrvAds1115ChannelMode channel_mode,
                               const DrvAds1115DataSampleRate data_sample_rate,
                               const DrvAds1115Gain gain, bool is_single_shot);
int DrvAds1115_StopConversion();
int DrvAds1115_ReadConversion(int16_t* value);
float DrvAds1115_ConvertToVoltage(int16_t value, const DrvAds1115Gain gain);

#endif  // DRV_ADS1115_H