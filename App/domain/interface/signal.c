#include "signal.h"

WEAK_LINK bool Signal_IsMainContactorOn(void) { return false; }

WEAK_LINK bool Signal_IsChargeContactorOn(void) { return false; }

WEAK_LINK bool Signal_IsDischargeContactorOn(void) { return false; }

WEAK_LINK bool Signal_IsMainPrechargeContactorOn(void) { return false; }

WEAK_LINK bool Signal_IsChargePrechargeContactorOn(void) { return false; }

WEAK_LINK bool Signal_IsDischargePrechargeContactorOn(void) { return false; }

WEAK_LINK bool Signal_IsChargeOn(void) { return false; }

WEAK_LINK bool Signal_IsWakeUpOn(void) { return false; }

WEAK_LINK bool Signal_IsForceContactOn(void) { return false; }

WEAK_LINK bool Signal_IsProtectionReleaseOn(void) { return false; }

WEAK_LINK bool Signal_IsRescueOn(void) { return false; }

WEAK_LINK bool Signal_IsBdiEnableOn(void) { return false; }

WEAK_LINK void Signal_RegisterBdiEnableOnCallback(void (*callback)(void), uint32_t debounce_tick) {
    (void)callback;
    (void)debounce_tick;
}

WEAK_LINK void Signal_DisableBdiEnableOnCallback(void) {}

WEAK_LINK void Signal_SetMainContactor(bool state) { (void)state; }

WEAK_LINK void Signal_SetMainPrechargeContactor(bool state) { (void)state; }

WEAK_LINK void Signal_SetChargeContactor(bool state) { (void)state; }

WEAK_LINK void Signal_SetChargePrechargeContactor(bool state) { (void)state; }

WEAK_LINK void Signal_SetDischargeContactor(bool state) { (void)state; }

WEAK_LINK void Signal_SetDischargePrechargeContactor(bool state) { (void)state; }

WEAK_LINK void Signal_SetSubContactor(bool state) { (void)state; }

WEAK_LINK void Signal_SetVoltageEnable(bool state) { (void)state; }

WEAK_LINK void Signal_SetCurrentEnable(bool state) { (void)state; }

WEAK_LINK void Signal_SetSocInternalIndicator1(bool state) { (void)state; }

WEAK_LINK void Signal_SetSocInternalIndicator2(bool state) { (void)state; }

WEAK_LINK void Signal_SetSocInternalIndicator3(bool state) { (void)state; }

WEAK_LINK void Signal_SetSocInternalIndicator4(bool state) { (void)state; }

WEAK_LINK void Signal_SetSocInternalIndicator5(bool state) { (void)state; }

WEAK_LINK void Signal_SetSocExternalIndicator1(bool state) { (void)state; }

WEAK_LINK void Signal_SetSocExternalIndicator2(bool state) { (void)state; }

WEAK_LINK void Signal_SetSocExternalIndicator3(bool state) { (void)state; }

WEAK_LINK void Signal_SetSocExternalIndicator4(bool state) { (void)state; }

WEAK_LINK void Signal_SetSocExternalIndicator5(bool state) { (void)state; }

WEAK_LINK void Signal_SetStateRedLed(bool state) { (void)state; }

WEAK_LINK void Signal_SetStateGreenLed(bool state) { (void)state; }

WEAK_LINK void Signal_SetStateBlueLed(bool state) { (void)state; }

WEAK_LINK void Signal_SetFan(bool state) { (void)state; }

WEAK_LINK void Signal_SetHeater(bool state) { (void)state; }

WEAK_LINK void Signal_SetBuzzer(bool state) { (void)state; }
