#include "bdi.h"

#include "../interface/bdi_can.h"
#include "../interface/bdi_i2c.h"

#define self (bdi_private)

static struct {
    AppErrorInfo error_info;

    BmsConfig* config;

    BmsBdiType bdi_type;
} bdi_private;

AppResult Bdi_Initialize(const BmsConfig* config) {
    if (config == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BDI_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    self.config = (BmsConfig*)config;

    self.bdi_type = config->misc.bdi.type;

    if (self.bdi_type == BMS_BDI_TYPE_DISABLE) {
        return APP_RESULT_OK;
    }

    if (self.bdi_type & BMS_BDI_TYPE_MZN58_CAN) {
        if (BdiCan_Initialize(config) != APP_RESULT_OK) {
            BdiCan_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    if (self.bdi_type & BMS_BDI_TYPE_LP5018_I2C) {
        if (BdiI2c_Initialize(config) != APP_RESULT_OK) {
            BdiI2c_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    return APP_RESULT_OK;
}

AppResult Bdi_Deinitialize(void) {
    if (self.bdi_type == BMS_BDI_TYPE_DISABLE) {
        return APP_RESULT_OK;
    }

    if (self.bdi_type & BMS_BDI_TYPE_MZN58_CAN) {
        if (BdiCan_Deinitialize() != APP_RESULT_OK) {
            BdiCan_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    if (self.bdi_type & BMS_BDI_TYPE_LP5018_I2C) {
        if (BdiI2c_Deinitialize() != APP_RESULT_OK) {
            BdiI2c_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    return APP_RESULT_OK;
}

void Bdi_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

void Bdi_SetType(BmsBdiType type) { self.bdi_type = type; }

AppResult Bdi_Process(const BmsData* data) {
    if (self.bdi_type == BMS_BDI_TYPE_DISABLE) {
        return APP_RESULT_OK;
    }

    if (self.bdi_type & BMS_BDI_TYPE_MZN58_CAN) {
        if (data == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BDI_CAN_INVALID_ARG);
            return APP_RESULT_INVALID_ARG;
        }

        if (BdiCan_Update(data) != APP_RESULT_OK) {
            if (BdiCan_Deinitialize() != APP_RESULT_OK) {
                BdiCan_GetLastErrorInfo(&self.error_info);
                return APP_RESULT_FAIL;
            }

            if (BdiCan_Initialize(self.config) != APP_RESULT_OK) {
                BdiCan_GetLastErrorInfo(&self.error_info);
                return APP_RESULT_FAIL;
            }

            return APP_RESULT_OK;
        }
    }

    if (self.bdi_type & BMS_BDI_TYPE_LP5018_I2C) {
        if (data == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BDI_INVALID_ARG);
            return APP_RESULT_INVALID_ARG;
        }

        if (BdiI2c_Update(data) != APP_RESULT_OK) {
            if (BdiI2c_Deinitialize() != APP_RESULT_OK) {
                BdiI2c_GetLastErrorInfo(&self.error_info);
                return APP_RESULT_FAIL;
            }

            if (BdiI2c_Initialize(self.config) != APP_RESULT_OK) {
                BdiI2c_GetLastErrorInfo(&self.error_info);
                return APP_RESULT_FAIL;
            }

            return APP_RESULT_OK;
        }
    }

    return APP_RESULT_OK;
}

void Bdi_Test(const BmsData* data) {
    if (self.bdi_type & BMS_BDI_TYPE_LP5018_I2C) {
        BdiI2c_Test(data);
    }
}