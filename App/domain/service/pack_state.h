#ifndef PACK_STATE_H
#define PACK_STATE_H

#include "../model/app_model.h"

AppResult PackState_Initialize(BmsConfig* config, void (*callback)(BmsPackState state));
void PackState_GetErrorInfo(AppErrorInfo* error_info);
BmsPackState PackState_ParseState(int16_t current, uint16_t pack_voltage,
                                  BmsProtectionState protection, BmsProtectionState pf,
                                  uint16_t soc, uint16_t min_bank_voltage);
void PackState_WakeUp(void);
BmsPackState PackState_GetCurrentState(void);

#endif