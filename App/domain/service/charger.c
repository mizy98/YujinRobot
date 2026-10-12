#include "charger.h"

#include "../interface/charger_can.h"

#define self (charger_private)

static struct {
    AppErrorInfo error_info;
} charger_private;

AppResult Charger_Initialize(const BmsConfig* config, const ChargerCallback* callback) {
    if (ChargerCan_Initialize(config, &(ChargerCanCallback){
                                          .onConnected = callback->onConnected,
                                          .onDisconnected = callback->onDisconnected,
                                          .onChargeStarted = callback->onChargeStarted,
                                          .onChargeFinished = callback->onChargeFinished,
                                      }) != APP_RESULT_OK) {
        ChargerCan_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult Charger_Deinitialize(void) {
    if (ChargerCan_Deinitialize() != APP_RESULT_OK) {
        ChargerCan_GetLastErrorInfo(&self.error_info);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void Charger_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult Charger_Process(const BmsData* data) { return ChargerCan_Process(data); }
