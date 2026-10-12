#ifndef SYSTEM_H
#define SYSTEM_H

#include "../model/app_model.h"

void System_Reset(void);
uint32_t System_GetUniqueId(uint8_t index);
void System_GetResetReason(AppResetReason* reset_reason, uint32_t* raw_reset_flag);
void System_ClearResetReason(void);

#endif