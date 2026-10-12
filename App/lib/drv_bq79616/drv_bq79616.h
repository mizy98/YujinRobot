#ifndef DRV_BQ79616_H_
#define DRV_BQ79616_H_

// Include
//=============================================================================================================================
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Macro definitions
//=============================================================================================================================
#define DRV_BQ79616_MAX_CELL_COUNT (16u)
#define DRV_BQ79616_MAX_GPIO_COUNT (8u)
#define DRV_BQ79616_RESET_DELAY_MS (40u)
#define DRV_BQ79616_INITIAL_DELAY_MS (200u)
#define DRV_BQ79616_MAX_DEVICE_ADDRESS (0x40)
#define DRV_BQ79616_BROADCAST_DEVICE_ADDRESS (DRV_BQ79616_MAX_DEVICE_ADDRESS)
#define DRV_BQ79616_MIN_OV_THRESHOLD (2.7)
#define DRV_BQ79616_MAX_OV_THRESHOLD (4.475)
#define DRV_BQ79616_MIN_UV_THRESHOLD (1.2)
#define DRV_BQ79616_MAX_UV_THRESHOLD (3.1)

// Type definitions
//=============================================================================================================================
typedef enum {
    DRV_BQ79616_RESULT_OK,
    DRV_BQ79616_RESULT_FAIL,
    DRV_BQ79616_RESULT_BUSY,
    DRV_BQ79616_RESULT_TIMEOUT,
    DRV_BQ79616_RESULT_INVALID_ARG
} DrvBq79616Result;

typedef enum {
    DRV_BQ79616_PING_SLEEP_TO_ACTIVE,
    DRV_BQ79616_PING_WAKE,
    DRV_BQ79616_PING_SHUTDOWN,
    DRV_BQ79616_PING_HW_RESET,
} DrvBq79616Ping;

typedef enum {                       // Appoximate settling time
    DRV_BQ79616_LPF_CUT_OFF_6_5_HZ,  // 152 ms
    DRV_BQ79616_LPF_CUT_OFF_13_HZ,   // 77 ms
    DRV_BQ79616_LPF_CUT_OFF_26_HZ,   // 38 ms
    DRV_BQ79616_LPF_CUT_OFF_53_HZ,   // 19 ms
    DRV_BQ79616_LPF_CUT_OFF_111_HZ,  // 9 ms
    DRV_BQ79616_LPF_CUT_OFF_240_HZ,  // 4 ms
    DRV_BQ79616_LPF_CUT_OFF_600_HZ,  // 1.6ms
} DrvBq79616LowPassFilter;

typedef enum {
    DRV_BQ79616_BALANCING_TIME_STOP,
    DRV_BQ79616_BALANCING_TIME_10_SEC,
    DRV_BQ79616_BALANCING_TIME_30_SEC,
    DRV_BQ79616_BALANCING_TIME_60_SEC,
    DRV_BQ79616_BALANCING_TIME_300_SEC,
    DRV_BQ79616_BALANCING_TIME_10_MIN,
    DRV_BQ79616_BALANCING_TIME_20_MIN,
    DRV_BQ79616_BALANCING_TIME_30_MIN,
    DRV_BQ79616_BALANCING_TIME_40_MIN,
    DRV_BQ79616_BALANCING_TIME_50_MIN,
    DRV_BQ79616_BALANCING_TIME_60_MIN,
    DRV_BQ79616_BALANCING_TIME_70_MIN,
    DRV_BQ79616_BALANCING_TIME_80_MIN,
    DRV_BQ79616_BALANCING_TIME_90_MIN,
    DRV_BQ79616_BALANCING_TIME_100_MIN,
    DRV_BQ79616_BALANCING_TIME_110_MIN,
    DRV_BQ79616_BALANCING_TIME_120_MIN,
    DRV_BQ79616_BALANCING_TIME_150_MIN,
    DRV_BQ79616_BALANCING_TIME_180_MIN,
    DRV_BQ79616_BALANCING_TIME_210_MIN,
    DRV_BQ79616_BALANCING_TIME_240_MIN,
    DRV_BQ79616_BALANCING_TIME_270_MIN,
    DRV_BQ79616_BALANCING_TIME_300_MIN,
    DRV_BQ79616_BALANCING_TIME_330_MIN,
    DRV_BQ79616_BALANCING_TIME_360_MIN,
    DRV_BQ79616_BALANCING_TIME_390_MIN,
    DRV_BQ79616_BALANCING_TIME_420_MIN,
    DRV_BQ79616_BALANCING_TIME_450_MIN,
    DRV_BQ79616_BALANCING_TIME_480_MIN,
    DRV_BQ79616_BALANCING_TIME_510_MIN,
    DRV_BQ79616_BALANCING_TIME_540_MIN,
    DRV_BQ79616_BALANCING_TIME_600_MIN
} DrvBq79616BalancingTime;

/*
000 = As disabled, high-Z
001 = As ADC and OTUT inputs
010 = As ADC only input
011 = As digital input
100 = As output high
101 = As output low
110 = As ADC input and weak pull-up enabled
111 = As ADC input and weak pull-down enabled
*/

typedef enum {
    DRV_BQ79616_GPIO_ROLE_DISABLED,
    DRV_BQ79616_GPIO_ROLE_ADC_AND_OTUT,
    DRV_BQ79616_GPIO_ROLE_ADC_ONLY,
    DRV_BQ79616_GPIO_ROLE_DIGITAL,
    DRV_BQ79616_GPIO_ROLE_OUTPUT_HIGH,
    DRV_BQ79616_GPIO_ROLE_OUTPUT_LOW,
    DRV_BQ79616_GPIO_ROLE_ADC_WEAK_PULL_UP,
    DRV_BQ79616_GPIO_ROLE_ADC_WEAK_PULL_DOWN
} DrvBq79616GpioRole;

#pragma pack(push, 1)

typedef struct {
    uint8_t active_cell_count;
    bool is_voltage_protector_enabled;
    bool is_temperature_protector_enabled;
    double under_voltage_protector_threshold;
    double over_voltage_protector_threshold;
    double under_temperature_protector_threshold;
    double over_temperature_protector_threshold;
    DrvBq79616LowPassFilter lpf_option;
    DrvBq79616GpioRole gpio_roles[DRV_BQ79616_MAX_GPIO_COUNT];
} DrvBq79616Config;

typedef struct {
    bool is_auto;
    bool is_manual_target_enabled[DRV_BQ79616_MAX_CELL_COUNT];
    uint8_t active_cell_count;
    double threshold_voltage;
    DrvBq79616BalancingTime time;
} DrvBq79616BalancingConfig;

typedef struct {
    bool is_main_adc_running;
    bool is_auxiliary_adc_running;
    bool is_voltage_protector_running;
    bool is_temperature_protector_running;
    bool is_balancing_running;
    bool is_balancing_pause;
    bool is_balancing_fault;
    bool is_balancing_over_temperature_detected;
    bool is_balancing_done[DRV_BQ79616_MAX_CELL_COUNT];
    uint32_t balancing_remaining_time[DRV_BQ79616_MAX_CELL_COUNT];
} DrvBq79616Status;

typedef struct {
    double die_temperature;
    double tsref_voltage;
    double cell_voltages[DRV_BQ79616_MAX_CELL_COUNT];
    double busbar_voltage;
    double gpio_voltages[DRV_BQ79616_MAX_GPIO_COUNT];
} DrvBq79616MainAdcValue;

typedef struct {
    bool (*isUartTxReady)();
    bool (*isUartRxReady)();
    DrvBq79616Result (*writeUart)(uint8_t* datas, uint8_t length);
    DrvBq79616Result (*listenUart)(uint8_t* datas, uint8_t length);
    void (*switchUartToGpio)(uint8_t tx_pin_state, uint8_t rx_pin_state);
    void (*switchGpioToUart)();
    void (*writeTxPin)(uint8_t pin_state);
    void (*writeRxPin)(uint8_t pin_state);
    void (*delayMs)(uint32_t ms);
    void (*delayUs)(uint32_t us);
    void (*delayCycle)();
} DrvBq79616Interface;

struct DrvBq79616Private;

typedef struct {
    // Methods
    DrvBq79616Result (*initialize)(DrvBq79616Interface* interface, uint8_t daisy_chain_count);
    DrvBq79616Result (*deinitialize)();
    void (*ping)(DrvBq79616Ping type);
    DrvBq79616Result (*setConfiguration)(uint8_t device_address, DrvBq79616Config* config);
    DrvBq79616Result (*getConfiguration)(uint8_t device_address, DrvBq79616Config* config);
    DrvBq79616Result (*startAdc)(uint8_t device_address, bool is_low_pass_filter_enabled,
                                 bool is_continuous);
    DrvBq79616Result (*stopAdc)(uint8_t device_address);
    DrvBq79616Result (*startBalancing)(uint8_t device_address, DrvBq79616BalancingConfig* config);
    DrvBq79616Result (*stopBalancing)(uint8_t device_address);
    DrvBq79616Result (*getMainAdcValue)(uint8_t device_address, DrvBq79616MainAdcValue* value);
    DrvBq79616Result (*getStatus)(uint8_t device_address, DrvBq79616Status* status);

    struct DrvBq79616Private* private;
} DrvBq79616;

#pragma pack(pop)

extern const DrvBq79616 drv_bq79616;

#endif
