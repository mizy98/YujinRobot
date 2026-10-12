#include "../../domain/interface/timer.h"
#include "../service/platform.h"

#define MAX_TIMER_COUNT (8u)

#define self (timer_private)

typedef struct {
    void* timer_id;
    void* context;
    void (*callback)(void* context);
} TimerHandle;

static struct {
    AppErrorInfo error_info;
    TimerHandle handles[MAX_TIMER_COUNT];
} timer_private;

TimerHandle* _getEmptyTimerHandle() {
    for (uint8_t i = 0; i < MAX_TIMER_COUNT; ++i) {
        if (self.handles[i].callback == NULL) {
            return &self.handles[i];
        }
    }
    return NULL;
}

bool _isValidTimerHandle(void* instance) {
    for (uint8_t i = 0; i < MAX_TIMER_COUNT; ++i) {
        if (&self.handles[i] == instance) {
            return true;
        }
    }
    return false;
}

AppResult Timer_Initialize(void) { return APP_RESULT_OK; }

AppResult Timer_Deinitialize(void) { return APP_RESULT_OK; }

void Timer_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

void* Timer_Create(void (*callback)(void* context), bool is_singleshot, void* context) {
    if (callback == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TIMER_INVALID_ARG);
        return NULL;
    }

    TimerHandle* handle = _getEmptyTimerHandle();
    if (handle == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TIMER_NO_AVAILABLE_HANDLE);
        return NULL;
    }

    if (handle->timer_id != NULL) {
        if (osTimerDelete(handle->timer_id) != osOK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TIMER_DRV_DELETE);
            return NULL;
        }
        handle->timer_id = NULL;
    }

    void* timer_id =
        osTimerNew(callback, is_singleshot ? osTimerOnce : osTimerPeriodic, context, NULL);
    if (timer_id == NULL) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TIMER_DRV_CREATE);
        return NULL;
    }

    handle->timer_id = timer_id;
    handle->callback = callback;
    handle->context = context;

    return handle;
}

AppResult Timer_Dispose(void* instance) {
    if (instance == NULL || !_isValidTimerHandle(instance)) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TIMER_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    TimerHandle* handle = (TimerHandle*)instance;

    if (handle->timer_id != NULL) {
        if (osTimerDelete(handle->timer_id) != osOK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TIMER_DRV_DELETE);
            return APP_RESULT_FAIL;
        }
        handle->timer_id = NULL;
    }

    handle->callback = NULL;
    handle->context = NULL;

    return APP_RESULT_OK;
}

AppResult Timer_Start(void* instance, uint32_t tick, bool is_singleshot, void* context) {
    (void)is_singleshot;
    (void)context;

    if (!_isValidTimerHandle(instance) || tick == 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TIMER_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    TimerHandle* handle = (TimerHandle*)instance;
    if (osTimerStart(handle->timer_id, tick) != osOK) {
        osTimerDelete(handle->timer_id);
        handle->timer_id = NULL;
        handle->context = NULL;

        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TIMER_DRV_START);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult Timer_Stop(void* instance) {
    if (!_isValidTimerHandle(instance)) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TIMER_INVALID_ARG);
        return APP_RESULT_FAIL;
    }

    TimerHandle* handle = (TimerHandle*)instance;
    handle->context = NULL;

    if (handle->timer_id != NULL) {
        if (osTimerStop(handle->timer_id) != osOK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TIMER_DRV_STOP);
            return APP_RESULT_FAIL;
        }
    }

    return APP_RESULT_OK;
}

bool Timer_IsRunning(void* instance) {
    if (!_isValidTimerHandle(instance)) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_TIMER_INVALID_ARG);
        return false;
    }

    TimerHandle* handle = (TimerHandle*)instance;
    return (bool)osTimerIsRunning(handle->timer_id);
}