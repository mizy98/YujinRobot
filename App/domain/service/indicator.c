#include "indicator.h"

#include "../interface/signal.h"
#include "../interface/system_time.h"

#define self (indicator_private)
#define BLINK_INTERVAL_MS (1000u)

typedef enum {
    SOC_STATE_0_5,
    SOC_STATE_5_20,
    SOC_STATE_20_40,
    SOC_STATE_40_60,
    SOC_STATE_60_80,
    SOC_STATE_80_95,
    SOC_STATE_95_100,
} SocIndicatorState;

static struct {
    struct {
        uint32_t blink_tick;  // ms
        bool blink_state;
    } pack_state;

    struct {
        SocIndicatorState state;
        uint32_t blink_tick;  // ms
        bool blink_state;
    } soc;

} indicator_private;

static void _processSoc(uint16_t soc) {
    uint32_t current_tick = SystemTime_GetTick();
    SocIndicatorState soc_state;

    if (soc >= 950) {
        soc_state = SOC_STATE_95_100;
    } else if (soc >= 800) {
        soc_state = SOC_STATE_80_95;
    } else if (soc >= 600) {
        soc_state = SOC_STATE_60_80;
    } else if (soc >= 400) {
        soc_state = SOC_STATE_40_60;
    } else if (soc >= 200) {
        soc_state = SOC_STATE_20_40;
    } else if (soc >= 50) {
        soc_state = SOC_STATE_5_20;
    } else {
        soc_state = SOC_STATE_0_5;
    }

    bool is_soc_state_changed = soc_state != self.soc.state;

    if (is_soc_state_changed) {
        self.soc.state = soc_state;
        self.soc.blink_tick = current_tick;
        self.soc.blink_state = true;
    }

    if (current_tick - self.soc.blink_tick >= BLINK_INTERVAL_MS) {
        self.soc.blink_state = !self.soc.blink_state;
        self.soc.blink_tick = current_tick;
    }

    switch (soc_state) {
        case SOC_STATE_0_5:
            Signal_SetSocInternalIndicator1(false);
            Signal_SetSocInternalIndicator2(false);
            Signal_SetSocInternalIndicator3(false);
            Signal_SetSocInternalIndicator4(false);
            Signal_SetSocInternalIndicator5(false);

            Signal_SetSocExternalIndicator1(false);
            Signal_SetSocExternalIndicator2(false);
            Signal_SetSocExternalIndicator3(false);
            Signal_SetSocExternalIndicator4(false);
            Signal_SetSocExternalIndicator5(false);
            break;

        case SOC_STATE_5_20:
            Signal_SetSocInternalIndicator1(self.soc.blink_state);
            Signal_SetSocInternalIndicator2(false);
            Signal_SetSocInternalIndicator3(false);
            Signal_SetSocInternalIndicator4(false);
            Signal_SetSocInternalIndicator5(false);

            Signal_SetSocExternalIndicator1(self.soc.blink_state);
            Signal_SetSocExternalIndicator2(false);
            Signal_SetSocExternalIndicator3(false);
            Signal_SetSocExternalIndicator4(false);
            Signal_SetSocExternalIndicator5(false);
            break;

        case SOC_STATE_20_40:
            Signal_SetSocInternalIndicator1(true);
            Signal_SetSocInternalIndicator2(self.soc.blink_state);
            Signal_SetSocInternalIndicator3(false);
            Signal_SetSocInternalIndicator4(false);
            Signal_SetSocInternalIndicator5(false);

            Signal_SetSocExternalIndicator1(true);
            Signal_SetSocExternalIndicator2(self.soc.blink_state);
            Signal_SetSocExternalIndicator3(false);
            Signal_SetSocExternalIndicator4(false);
            Signal_SetSocExternalIndicator5(false);
            break;

        case SOC_STATE_40_60:
            Signal_SetSocInternalIndicator1(true);
            Signal_SetSocInternalIndicator2(true);
            Signal_SetSocInternalIndicator3(self.soc.blink_state);
            Signal_SetSocInternalIndicator4(false);
            Signal_SetSocInternalIndicator5(false);

            Signal_SetSocExternalIndicator1(true);
            Signal_SetSocExternalIndicator2(true);
            Signal_SetSocExternalIndicator3(self.soc.blink_state);
            Signal_SetSocExternalIndicator4(false);
            Signal_SetSocExternalIndicator5(false);
            break;

        case SOC_STATE_60_80:
            Signal_SetSocInternalIndicator1(true);
            Signal_SetSocInternalIndicator2(true);
            Signal_SetSocInternalIndicator3(true);
            Signal_SetSocInternalIndicator4(self.soc.blink_state);
            Signal_SetSocInternalIndicator5(false);

            Signal_SetSocExternalIndicator1(true);
            Signal_SetSocExternalIndicator2(true);
            Signal_SetSocExternalIndicator3(true);
            Signal_SetSocExternalIndicator4(self.soc.blink_state);
            Signal_SetSocExternalIndicator5(false);
            break;

        case SOC_STATE_80_95:
            Signal_SetSocInternalIndicator1(true);
            Signal_SetSocInternalIndicator2(true);
            Signal_SetSocInternalIndicator3(true);
            Signal_SetSocInternalIndicator4(true);
            Signal_SetSocInternalIndicator5(self.soc.blink_state);

            Signal_SetSocExternalIndicator1(true);
            Signal_SetSocExternalIndicator2(true);
            Signal_SetSocExternalIndicator3(true);
            Signal_SetSocExternalIndicator4(true);
            Signal_SetSocExternalIndicator5(self.soc.blink_state);
            break;

        case SOC_STATE_95_100:
            Signal_SetSocInternalIndicator1(true);
            Signal_SetSocInternalIndicator2(true);
            Signal_SetSocInternalIndicator3(true);
            Signal_SetSocInternalIndicator4(true);
            Signal_SetSocInternalIndicator5(true);

            Signal_SetSocExternalIndicator1(true);
            Signal_SetSocExternalIndicator2(true);
            Signal_SetSocExternalIndicator3(true);
            Signal_SetSocExternalIndicator4(true);
            Signal_SetSocExternalIndicator5(true);
            break;

        default:
            break;
    }
}

static void _processState(BmsPackState state) {
    uint32_t current_tick = SystemTime_GetTick();

    if (current_tick - self.pack_state.blink_tick >= BLINK_INTERVAL_MS) {
        self.pack_state.blink_state = !self.pack_state.blink_state;
        self.pack_state.blink_tick = current_tick;
    }

    switch (state) {
        case BMS_PACK_STATE_IDLE:
            // Green blinking
            Signal_SetStateRedLed(false);
            Signal_SetStateGreenLed(self.pack_state.blink_state);
            Signal_SetStateBlueLed(false);
            break;

        case BMS_PACK_STATE_CHARGE:
        case BMS_PACK_STATE_CHARGE_CC:
        case BMS_PACK_STATE_CHARGE_CV:
            // Yellow (= Red + Green) steady
            Signal_SetStateRedLed(true);
            Signal_SetStateGreenLed(true);
            Signal_SetStateBlueLed(false);
            break;

        case BMS_PACK_STATE_DISCHARGE:
            // Blue steady
            Signal_SetStateRedLed(false);
            Signal_SetStateGreenLed(false);
            Signal_SetStateBlueLed(true);
            break;

        case BMS_PACK_STATE_SLEEP:
            // All off
            Signal_SetStateRedLed(false);
            Signal_SetStateGreenLed(false);
            Signal_SetStateBlueLed(false);
            break;

        case BMS_PACK_STATE_PROTECTION:
            // Red blinking
            Signal_SetStateRedLed(self.pack_state.blink_state);
            Signal_SetStateGreenLed(false);
            Signal_SetStateBlueLed(false);
            break;

        case BMS_PACK_STATE_PERMANENT_FAILURE:
            // Red steady
            Signal_SetStateRedLed(true);
            Signal_SetStateGreenLed(false);
            Signal_SetStateBlueLed(false);
            break;

        case BMS_PACK_STATE_REGENERATE:
            // Purple (= Blue + Red) steady
            Signal_SetStateRedLed(true);
            Signal_SetStateGreenLed(false);
            Signal_SetStateBlueLed(true);
            break;

        default:
            // Unknown state - all off
            Signal_SetStateRedLed(false);
            Signal_SetStateGreenLed(false);
            Signal_SetStateBlueLed(false);
            break;
    }
}

void Indicator_Initialize() {
    self.soc.state = SOC_STATE_0_5;
    self.soc.blink_tick = 0;
    self.soc.blink_state = true;

    self.pack_state.blink_tick = 0;
    self.pack_state.blink_state = true;
}

void Indicator_Process(BmsPackState state, uint16_t soc) {
    _processSoc(soc);
    _processState(state);
}
