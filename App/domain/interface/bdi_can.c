#include "bdi_can.h"

WEAK_LINK AppResult BdiCan_Initialize(const BmsConfig* config) {
    (void)config;
    return APP_RESULT_FAIL;
}
WEAK_LINK AppResult BdiCan_Deinitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void BdiCan_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        info->code = APP_ERROR_NOT_IMPLEMENT;
    }
}

WEAK_LINK AppResult BdiCan_Update(const BmsData* data) { return APP_RESULT_FAIL; }