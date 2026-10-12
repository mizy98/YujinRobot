#include "bdi_i2c.h"

WEAK_LINK AppResult BdiI2c_Initialize(const BmsConfig* config) {
    (void)config;
    return APP_RESULT_FAIL;
}
WEAK_LINK AppResult BdiI2c_Deinitialize(void) { return APP_RESULT_FAIL; }

WEAK_LINK void BdiI2c_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        info->code = APP_ERROR_NOT_IMPLEMENT;
    }
}

WEAK_LINK AppResult BdiI2c_Update(const BmsData* data) { return APP_RESULT_FAIL; }

WEAK_LINK void BdiI2c_Test(const BmsData* data) { (void)data; }