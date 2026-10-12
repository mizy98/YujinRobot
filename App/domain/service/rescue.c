#include "rescue.h"

#include "../interface/signal.h"
#include "../interface/system_time.h"

#define self (rescue_private)

#define RESCUE_ACTIVATION_TICK (1000u)
#define RESCUE_DEACTIVATION_TICK (1000u)

static struct {
    uint32_t check_start_tick;

    bool is_rescue_on;

    RescueCallback callback;

    AppErrorInfo error_info;
} rescue_private;

AppResult Rescue_Initialize(const RescueCallback* callback) {
    if (callback == NULL || callback->onActivated == NULL || callback->onDeactivated == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_RESCUE_INVALID_ARG);
        return APP_RESULT_INVALID_ARG;
    }

    self.callback = *callback;

    return APP_RESULT_OK;
}

AppResult Rescue_Deinitialize(void) { return APP_RESULT_OK; }

void Rescue_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

void Rescue_Process(void) {
    uint32_t current_tick = SystemTime_GetTick();

    if (self.is_rescue_on) {
        if (Signal_IsRescueOn() == false) {
            if (self.check_start_tick == 0) {
                self.check_start_tick = current_tick;
            } else if ((current_tick - self.check_start_tick) >= RESCUE_DEACTIVATION_TICK) {
                self.is_rescue_on = false;
                self.check_start_tick = 0;

                if (self.callback.onDeactivated != NULL) {
                    self.callback.onDeactivated();
                }
            }
        } else {
            self.check_start_tick = 0;
        }
    } else {
        if (Signal_IsRescueOn() == true) {
            if (self.check_start_tick == 0) {
                self.check_start_tick = current_tick;
            } else if ((current_tick - self.check_start_tick) >= RESCUE_ACTIVATION_TICK) {
                self.is_rescue_on = true;
                self.check_start_tick = 0;

                if (self.callback.onActivated != NULL) {
                    self.callback.onActivated();
                }
            }
        } else {
            self.check_start_tick = 0;
        }
    }
}
