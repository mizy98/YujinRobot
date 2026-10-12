#include "charger_can.h"

WEAK_LINK AppResult ChargerCan_Initialize(const BmsConfig* config,
                                          const ChargerCanCallback* callback) {
    (void)config;
    return APP_RESULT_FAIL;
}

WEAK_LINK AppResult ChargerCan_Deinitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void ChargerCan_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        info->code = APP_ERROR_NOT_IMPLEMENT;
    }
}

WEAK_LINK AppResult ChargerCan_Process(const BmsData* data) { (void)data; return APP_RESULT_FAIL; }

WEAK_LINK ChargerCanState ChargerCan_GetState() { return CHARGER_CAN_STATE_IDLE; }

WEAK_LINK void ChargerCan_GetStatus(ChargerCanStatus* status) { (void)status; }
