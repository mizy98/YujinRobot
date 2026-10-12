#ifndef THRESHOLD_CONTROL_H
#define THRESHOLD_CONTROL_H

#include "../model/app_model.h"

#define THRESHOLD_CONTROL_MAX_HANDLE 32

typedef struct {
    bool is_enabled;
    bool is_over_trigger;
    bool off_enabled;
    int32_t on_threshold;
    int32_t off_threshold;
    uint32_t on_duration_tick;
    uint32_t off_duration_tick;
} ThresholdControlConfig;

typedef struct {
    void (*onStateChanged)(void* context, bool is_active, int32_t snapshot_value);
} ThresholdControlCallback;

typedef struct {
    uint32_t (*getSystemTick)(void);
} ThresholdControlInterface;

int ThresholdControl_Initialize(ThresholdControlInterface* itf);
void* ThresholdControl_Create(ThresholdControlConfig* config, ThresholdControlCallback* callback,
                              void* context, bool initial_is_active);
void ThresholdControl_Destroy(void* handle);
void ThresholdControl_CheckValue(void* handle, int32_t value);
bool ThresholdControl_IsActive(void* handle);
int ThresholdControl_UpdateConfig(void* handle, ThresholdControlConfig* config);
void ThresholdControl_ResetDuration(void* handle);
void ThresholdControl_Reset(void* handle);
bool ThresholdControl_IsTriggered(void* handle, int32_t value);

#endif