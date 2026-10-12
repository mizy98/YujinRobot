#ifndef DRV_LP5018_H
#define DRV_LP5018_H

#include "drv_lp5018_define.h"

int DrvLp5018_Initialize(DrvLp5018Config* config, DrvLp5018Interface* interface);
int DrvLp5018_Reset(void);
int DrvLp5018_SetOutput(DrvLp5018Output output, uint8_t brightness);
int DrvLp5018_SetLed(DrvLp5018Led led, uint8_t brightness, uint8_t red, uint8_t green,
                     uint8_t blue);

#endif