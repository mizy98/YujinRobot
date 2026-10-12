#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "../model/app_model.h"

AppResult Temperature_Initialize(const BmsConfig* config);
AppResult Temperature_DeInitialize(void);
void Temperature_GetLastErrorInfo(AppErrorInfo* error_info);
AppResult Temperature_GetDatas(BmsPackTemperatureData* pack_data,
                               BmsBoardTemperatureData* board_data,
                               BmsBalancerTemperatureData* balancer_data,
                               BmsContactorTemperatureData* contactor_data);
AppResult Temperature_GetPackData(BmsPackTemperatureData* data);
AppResult Temperature_GetBoardData(BmsBoardTemperatureData* data);
AppResult Temperature_GetBalancerData(BmsBalancerTemperatureData* data);
AppResult Temperature_GetContactorData(BmsContactorTemperatureData* data);

#endif