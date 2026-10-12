#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <stdio.h>
#include <string.h>

#include "../model/app_model.h"

AppResult Configurator_Initialize();
AppResult Configurator_Deinitialize();
void Configurator_GetLastErrorInfo(AppErrorInfo* info);

AppResult Configurator_Set(const BmsConfig* config);
void Configurator_Get(BmsConfig* config);

AppResult Configurator_Save(const BmsConfig* config);
AppResult Configurator_Load(BmsConfig* config);
AppResult Configurator_EraseAll(void);

#endif
