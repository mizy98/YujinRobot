#ifndef NVS_H
#define NVS_H

#include "../model/app_model.h"

AppResult Nvs_Initialize(void);
AppResult Nvs_Deinitialize(void);
void Nvs_GetLastErrorInfo(AppErrorInfo* error_info);
bool Nvs_IsFormatOccurred(void);
AppResult Nvs_SaveSoc(uint32_t timestamp, uint16_t soc);
AppResult Nvs_LoadSoc(uint32_t* timestamp, uint16_t* soc);
AppResult Nvs_SaveCycle(float cycle);
AppResult Nvs_LoadCycle(float* cycle);
AppResult Nvs_SaveSoh(float soh);
AppResult Nvs_LoadSoh(float* soh);
AppResult Nvs_SavePfState(uint32_t pf_state);
AppResult Nvs_LoadPfState(uint32_t* pf_state);

#endif