#ifndef PROTECTION_H
#define PROTECTION_H

#include "../model/app_model.h"

typedef struct {
    void (*onProtectionOccurred)(BmsProtectionCode code, int32_t value, int32_t threshold);
    void (*onProtectionReleased)(BmsProtectionCode code, int32_t value, int32_t threshold);
    void (*onPermanentFailure)(BmsProtectionCode code, int32_t value, int32_t threshold);
    void (*onPermanentFailureReleased)(BmsProtectionCode code);
} ProtectionCallback;

AppResult Protection_Initialize(BmsConfig* config, BmsProtectionState pf_state,
                                ProtectionCallback* callback);
void Protection_GetLastErrorInfo(AppErrorInfo* error_info);
AppResult Protection_UpdateConfig(BmsConfig* config);
void Protection_Check(BmsData* data);
void Protection_SetPfState(BmsProtectionState pf_state);
void Protection_GetPfState(BmsProtectionState* pf_state);
void Protection_ResetPfState(void);
bool Protection_IsTriggered(BmsProtectionCode code, BmsData* data);
void Protection_Reset();
void Protection_GetError(AppErrorInfo* error);

#endif