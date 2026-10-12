#include "threshold_control.h"

typedef struct {
    bool active;
    uint32_t start_tick;
} DurationGuard;

typedef struct {
    bool in_use;
    bool is_active;
    DurationGuard on_guard;
    DurationGuard off_guard;

    void* context;

    ThresholdControlConfig config;
    ThresholdControlCallback callback;
} ThresholdControlHandle;

static ThresholdControlInterface _itf;
static ThresholdControlHandle _handles[THRESHOLD_CONTROL_MAX_HANDLE] = {0};

static bool ThresholdControl_IsValidHandle(void* handle) {
    if (handle == NULL) {
        return false;
    }

    ThresholdControlHandle* th_handle = (ThresholdControlHandle*)handle;

    if (th_handle < _handles || th_handle >= (_handles + THRESHOLD_CONTROL_MAX_HANDLE)) {
        return false;
    }

    if (!th_handle->in_use) {
        return false;
    }

    return true;
}

static bool ThresholdControl_IsValidConfig(ThresholdControlConfig* config) {
    if (config == NULL) {
        return false;
    }

    if (!config->off_enabled) {
        return true;
    }

    return true;
}

static bool _isDurationExpired(DurationGuard* guard, uint32_t now, uint32_t duration_ticks) {
    if (duration_ticks == 0) {
        return true;
    }

    if (!guard->active) {
        guard->active = true;
        guard->start_tick = now;
        return false;
    }

    return (uint32_t)(now - guard->start_tick) >= duration_ticks;
}

static void _resetDuration(DurationGuard* guard) {
    guard->active = false;
    guard->start_tick = 0;
}

int ThresholdControl_Initialize(ThresholdControlInterface* itf) {
    if (itf == NULL || itf->getSystemTick == NULL) {
        return -1;
    }

    _itf = *itf;

    for (uint8_t i = 0; i < THRESHOLD_CONTROL_MAX_HANDLE; i++) {
        _handles[i].in_use = false;
        _handles[i].is_active = false;
        _handles[i].context = NULL;
        _handles[i].callback.onStateChanged = NULL;
        _resetDuration(&_handles[i].on_guard);
        _resetDuration(&_handles[i].off_guard);
    }

    return 0;
}

void* ThresholdControl_Create(ThresholdControlConfig* config, ThresholdControlCallback* callback,
                              void* context, bool initial_is_active) {
    if (!ThresholdControl_IsValidConfig(config)) {
        return NULL;
    }

    for (uint8_t i = 0; i < THRESHOLD_CONTROL_MAX_HANDLE; i++) {
        if (!_handles[i].in_use) {
            ThresholdControlHandle* handle = &_handles[i];

            handle->in_use = true;
            handle->config = *config;
            handle->context = context;
            handle->is_active = initial_is_active;

            if (callback) {
                handle->callback = *callback;
            } else {
                handle->callback.onStateChanged = NULL;
            }

            _resetDuration(&handle->on_guard);
            _resetDuration(&handle->off_guard);

            return (void*)handle;
        }
    }

    return NULL;
}

void ThresholdControl_Destroy(void* handle) {
    if (handle == NULL) {
        return;
    }

    ThresholdControlHandle* th_handle = (ThresholdControlHandle*)handle;

    if (th_handle < _handles || th_handle >= (_handles + THRESHOLD_CONTROL_MAX_HANDLE)) {
        return;
    }

    if (!th_handle->in_use) {
        return;
    }

    th_handle->in_use = false;
    th_handle->is_active = false;
    th_handle->context = NULL;
    th_handle->callback.onStateChanged = NULL;

    _resetDuration(&th_handle->on_guard);
    _resetDuration(&th_handle->off_guard);
}

void ThresholdControl_CheckValue(void* handle, int32_t value) {
    if (!ThresholdControl_IsValidHandle(handle)) {
        return;
    }

    ThresholdControlHandle* th_handle = (ThresholdControlHandle*)handle;

    if (!th_handle->config.is_enabled) {
        return;
    }

    uint32_t now = _itf.getSystemTick();
    bool should_activate = false;
    bool should_deactivate = false;

    if (th_handle->config.is_over_trigger) {
        if (th_handle->is_active) {
            if (th_handle->config.off_enabled && value < th_handle->config.off_threshold) {
                if (_isDurationExpired(&th_handle->off_guard, now,
                                       th_handle->config.off_duration_tick)) {
                    should_deactivate = true;
                }
            } else {
                _resetDuration(&th_handle->off_guard);
            }
        } else {
            if (value > th_handle->config.on_threshold) {
                if (_isDurationExpired(&th_handle->on_guard, now,
                                       th_handle->config.on_duration_tick)) {
                    should_activate = true;
                }
            } else {
                _resetDuration(&th_handle->on_guard);
            }
        }
    } else {
        if (th_handle->is_active) {
            if (th_handle->config.off_enabled && value > th_handle->config.off_threshold) {
                if (_isDurationExpired(&th_handle->off_guard, now,
                                       th_handle->config.off_duration_tick)) {
                    should_deactivate = true;
                }
            } else {
                _resetDuration(&th_handle->off_guard);
            }
        } else {
            if (value < th_handle->config.on_threshold) {
                if (_isDurationExpired(&th_handle->on_guard, now,
                                       th_handle->config.on_duration_tick)) {
                    should_activate = true;
                }
            } else {
                _resetDuration(&th_handle->on_guard);
            }
        }
    }

    if (should_activate) {
        th_handle->is_active = true;
        _resetDuration(&th_handle->on_guard);
        if (th_handle->callback.onStateChanged) {
            th_handle->callback.onStateChanged(th_handle->context, true, value);
        }
    } else if (should_deactivate) {
        th_handle->is_active = false;
        _resetDuration(&th_handle->off_guard);
        if (th_handle->callback.onStateChanged) {
            th_handle->callback.onStateChanged(th_handle->context, false, value);
        }
    }
}

bool ThresholdControl_IsActive(void* handle) {
    if (!ThresholdControl_IsValidHandle(handle)) {
        return false;
    }

    ThresholdControlHandle* th_handle = (ThresholdControlHandle*)handle;
    return th_handle->is_active;
}

int ThresholdControl_UpdateConfig(void* handle, ThresholdControlConfig* config) {
    if (!ThresholdControl_IsValidHandle(handle) || !ThresholdControl_IsValidConfig(config)) {
        return -1;
    }

    ThresholdControlHandle* th_handle = (ThresholdControlHandle*)handle;

    th_handle->config = *config;

    th_handle->is_active = false;
    _resetDuration(&th_handle->on_guard);
    _resetDuration(&th_handle->off_guard);

    return 0;
}

void ThresholdControl_ResetDuration(void* handle) {
    if (!ThresholdControl_IsValidHandle(handle)) {
        return;
    }

    ThresholdControlHandle* th_handle = (ThresholdControlHandle*)handle;

    _resetDuration(&th_handle->on_guard);
    _resetDuration(&th_handle->off_guard);
}

void ThresholdControl_Reset(void* handle) {
    if (!ThresholdControl_IsValidHandle(handle)) {
        return;
    }

    ThresholdControlHandle* th_handle = (ThresholdControlHandle*)handle;

    th_handle->is_active = false;
    _resetDuration(&th_handle->on_guard);
    _resetDuration(&th_handle->off_guard);
}

bool ThresholdControl_IsTriggered(void* handle, int32_t value) {
    if (!ThresholdControl_IsValidHandle(handle)) {
        return false;
    }

    ThresholdControlHandle* th_handle = (ThresholdControlHandle*)handle;
    if (!th_handle->config.is_enabled) {
        return false;
    }

    if (th_handle->config.is_over_trigger) {
        return value > th_handle->config.on_threshold;
    } else {
        return value < th_handle->config.on_threshold;
    }
}
