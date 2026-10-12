#ifndef SIGNAL_H
#define SIGNAL_H

#include "../model/app_model.h"

// =========== input signals ===========
bool Signal_IsMainContactorOn(void);
bool Signal_IsChargeContactorOn(void);
bool Signal_IsDischargeContactorOn(void);

bool Signal_IsMainPrechargeContactorOn(void);
bool Signal_IsChargePrechargeContactorOn(void);
bool Signal_IsDischargePrechargeContactorOn(void);

bool Signal_IsChargeOn(void);
bool Signal_IsWakeUpOn(void);
bool Signal_IsForceContactOn(void);
bool Signal_IsProtectionReleaseOn(void);
bool Signal_IsRescueOn(void);
bool Signal_IsBdiEnableOn(void);

void Signal_RegisterBdiEnableOnCallback(void (*callback)(void), uint32_t debounce_tick);
void Signal_DisableBdiEnableOnCallback(void);

// =========== output signals ===========
void Signal_SetMainContactor(bool state);
void Signal_SetMainPrechargeContactor(bool state);
void Signal_SetChargeContactor(bool state);
void Signal_SetChargePrechargeContactor(bool state);
void Signal_SetDischargeContactor(bool state);
void Signal_SetDischargePrechargeContactor(bool state);
void Signal_SetSubContactor(bool state);

void Signal_SetVoltageEnable(bool state);
void Signal_SetCurrentEnable(bool state);

void Signal_SetSocInternalIndicator1(bool state);
void Signal_SetSocInternalIndicator2(bool state);
void Signal_SetSocInternalIndicator3(bool state);
void Signal_SetSocInternalIndicator4(bool state);
void Signal_SetSocInternalIndicator5(bool state);

void Signal_SetSocExternalIndicator1(bool state);
void Signal_SetSocExternalIndicator2(bool state);
void Signal_SetSocExternalIndicator3(bool state);
void Signal_SetSocExternalIndicator4(bool state);
void Signal_SetSocExternalIndicator5(bool state);

void Signal_SetStateRedLed(bool state);
void Signal_SetStateGreenLed(bool state);
void Signal_SetStateBlueLed(bool state);

void Signal_SetFan(bool state);
void Signal_SetHeater(bool state);

void Signal_SetBuzzer(bool state);

#endif