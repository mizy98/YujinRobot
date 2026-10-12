#ifndef INDICATOR_H
#define INDICATOR_H

#include "../interface/system_time.h"
#include "../model/app_model.h"

void Indicator_Initialize();
void Indicator_Process(BmsPackState state, uint16_t soc);

#endif