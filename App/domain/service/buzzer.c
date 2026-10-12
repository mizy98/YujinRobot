#include "buzzer.h"

#include <string.h>

#include "../interface/rtos.h"
#include "../interface/signal.h"
#include "../interface/timer.h"

#define self (buzzer_private)

#define MUTEX_TIMEOUT_TICK (100u)

typedef struct {
    uint32_t on_tick;
    uint32_t off_tick;
} BuzzerPulse;

static const BuzzerPulse pulse_init_completed[] = {{500, 0}, {0, 0}};
static const BuzzerPulse pulse_protection[] = {{30u * 1000u, 0}, {0, 0}};
static const BuzzerPulse pulse_storage_updated[] = {{135, 80}, {100, 0}};
static const BuzzerPulse pulse_voltage_error[] = {{100, 200}, {0, 0}};
static const BuzzerPulse pulse_current_error[] = {{100, 200}, {100, 200}, {0, 0}};
static const BuzzerPulse pulse_relay_error[] = {
    {100, 150},
    {100, 150},
    {100, 1850},
};
static const BuzzerPulse pulse_rescue_activated[] = {
    {100, 150},
    {100, 150},
    {100, 150},
    {100, 1850},
};

typedef struct {
    const BuzzerPulse* pattern;
    bool is_continuous;
} BuzzerPatternInfo;

static const BuzzerPatternInfo buzzer_patterns[] = {
    [BUZZER_TYPE_NONE] = {NULL, false},
    [BUZZER_TYPE_INIT_COMPLETED] = {pulse_init_completed, false},
    [BUZZER_TYPE_PROTECTION] = {pulse_protection, false},
    [BUZZER_TYPE_STORAGE_UPDATED] = {pulse_storage_updated, false},
    [BUZZER_TYPE_VOLTAGE_ERROR] = {pulse_voltage_error, true},
    [BUZZER_TYPE_CURRENT_ERROR] = {pulse_current_error, true},
    [BUZZER_TYPE_RELAY_ERROR] = {pulse_relay_error, true},
    [BUZZER_TYPE_RESCUE_ACTIVATED] = {pulse_rescue_activated, true},
};

typedef enum {
    BUZZER_STATE_IDLE,
    BUZZER_STATE_ON,
    BUZZER_STATE_OFF,
} BuzzerState;

static struct {
    BuzzerType current_type;
    BuzzerState state;

    const BuzzerPulse* pattern;
    uint8_t pulse_index;
    bool is_continuous;

    void* timer;
    void* mutex;

    AppErrorInfo error_info;
} buzzer_private;

static void _onTimer(void* context);
static void _startPulse(uint8_t index);
static void _stopLocked(void);

static void _stopLocked(void) {
    Signal_SetBuzzer(false);
    Timer_Stop(self.timer);
    self.state = BUZZER_STATE_IDLE;
    self.current_type = BUZZER_TYPE_NONE;
    self.pattern = NULL;
}

static void _startPulse(uint8_t index) {
    const BuzzerPulse* pulse = &self.pattern[index];

    if (pulse->on_tick == 0) {
        if (self.is_continuous) {
            index = 0;
            pulse = &self.pattern[0];
        } else {
            _stopLocked();
            return;
        }
    }

    self.pulse_index = index;
    self.state = BUZZER_STATE_ON;

    Signal_SetBuzzer(true);
    Timer_Start(self.timer, pulse->on_tick, true, NULL);
}

static void _onTimer(void* context) {
    (void)context;

    if (self.state == BUZZER_STATE_IDLE || self.pattern == NULL) {
        return;
    }

    const BuzzerPulse* pulse = &self.pattern[self.pulse_index];

    if (self.state == BUZZER_STATE_ON) {
        Signal_SetBuzzer(false);

        if (pulse->off_tick > 0) {
            self.state = BUZZER_STATE_OFF;
            Timer_Start(self.timer, pulse->off_tick, true, NULL);
        } else {
            _startPulse(self.pulse_index + 1);
        }

    } else if (self.state == BUZZER_STATE_OFF) {
        _startPulse(self.pulse_index + 1);
    }
}

AppResult Buzzer_Initialize(void) {
    memset(&self, 0, sizeof(self));
    Signal_SetBuzzer(false);

    if (self.mutex == NULL) {
        void* mutex = Rtos_CreateMutex();
        if (mutex == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BUZZER_MUTEX_CREATE);
            return APP_RESULT_FAIL;
        }
        self.mutex = mutex;
    }

    if (Rtos_LockMutex(self.mutex, MUTEX_TIMEOUT_TICK) != APP_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BUZZER_MUTEX_LOCK);

        Rtos_DeleteMutex(self.mutex);
        self.mutex = NULL;

        return APP_RESULT_FAIL;
    }

    if (self.timer == NULL) {
        void* timer = Timer_Create(_onTimer, true, NULL);
        if (timer == NULL) {
            Timer_GetLastErrorInfo(&self.error_info);

            Rtos_DeleteMutex(self.mutex);
            self.mutex = NULL;

            return APP_RESULT_FAIL;
        }

        self.timer = timer;
    }

    if (Rtos_UnlockMutex(self.mutex) != APP_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BUZZER_MUTEX_UNLOCK);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult Buzzer_Deinitialize(void) {
    if (Rtos_LockMutex(self.mutex, MUTEX_TIMEOUT_TICK) != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    _stopLocked();
    Timer_Dispose(self.timer);
    self.timer = NULL;

    Rtos_UnlockMutex(self.mutex);
    Rtos_DeleteMutex(self.mutex);
    self.mutex = NULL;

    return APP_RESULT_OK;
}

void Buzzer_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult Buzzer_Beep(BuzzerType type) {
    if (type == BUZZER_TYPE_NONE || type >= sizeof(buzzer_patterns) / sizeof(buzzer_patterns[0])) {
        return APP_RESULT_INVALID_ARG;
    }

    if (Rtos_LockMutex(self.mutex, MUTEX_TIMEOUT_TICK) != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    _stopLocked();

    const BuzzerPatternInfo* info = &buzzer_patterns[type];
    self.current_type = type;
    self.pattern = info->pattern;
    self.is_continuous = info->is_continuous;

    _startPulse(0);

    if (Rtos_UnlockMutex(self.mutex) != APP_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BUZZER_MUTEX_UNLOCK);
        return APP_RESULT_FAIL;
    }
    return APP_RESULT_OK;
}

AppResult Buzzer_Stop(void) {
    if (Rtos_LockMutex(self.mutex, MUTEX_TIMEOUT_TICK) != APP_RESULT_OK) {
        return APP_RESULT_FAIL;
    }

    _stopLocked();

    if (Rtos_UnlockMutex(self.mutex) != APP_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BUZZER_MUTEX_UNLOCK);
        return APP_RESULT_FAIL;
    }
    return APP_RESULT_OK;
}

BuzzerType Buzzer_GetCurrentType(void) {
    BuzzerType type;

    if (Rtos_LockMutex(self.mutex, MUTEX_TIMEOUT_TICK) != APP_RESULT_OK) {
        return BUZZER_TYPE_NONE;
    }

    type = self.current_type;

    if (Rtos_UnlockMutex(self.mutex) != APP_RESULT_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_BUZZER_MUTEX_UNLOCK);
        return BUZZER_TYPE_NONE;
    }

    return type;
}