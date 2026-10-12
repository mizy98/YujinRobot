#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "../model/app_model.h"

AppResult Validator_ValidateBmsConfig(const BmsConfig* config, AppErrorInfo* error_info);

#endif