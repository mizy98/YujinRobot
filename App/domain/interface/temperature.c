#include "temperature.h"

WEAK_LINK AppResult Temperature_Initialize(const BmsConfig* config) { return APP_RESULT_FAIL; }

WEAK_LINK AppResult Temperature_DeInitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void Temperature_GetLastErrorInfo(AppErrorInfo* error_info) {
    if (error_info != NULL) {
        error_info->code = APP_ERROR_NOT_IMPLEMENT;
    }
}

WEAK_LINK AppResult Temperature_GetDatas(BmsPackTemperatureData* pack_data,
                                         BmsBoardTemperatureData* board_data,
                                         BmsBalancerTemperatureData* balancer_data,
                                         BmsContactorTemperatureData* contactor_data) {
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult Temperature_GetPackData(BmsPackTemperatureData* data) {
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult Temperature_GetBoardData(BmsBoardTemperatureData* data) {
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult Temperature_GetBalancerData(BmsBalancerTemperatureData* data) {
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult Temperature_GetContactorData(BmsContactorTemperatureData* data) {
    return APP_RESULT_FAIL;
}