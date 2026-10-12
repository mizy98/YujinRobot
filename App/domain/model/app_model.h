#ifndef APP_MODEL_H
#define APP_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "bms_config.h"
#include "bms_data.h"
#include "bms_info.h"
#include "ll/app_boot_info.h"
#include "ll/app_config.h"
#include "ll/app_error.h"
#include "ll/app_type.h"
#include "ll/bms_data_log_entry.h"
#include "ll/bms_data_log_info.h"
#include "ll/bms_event_log_entry.h"
#include "ll/bms_expansion_data.h"

#pragma pack(push, 1)
typedef struct {
    AppBootInfo boot_info;
    BmsInfo info;
    BmsConfig config;
    BmsData data;
} App;
#pragma pack(pop)

#endif
