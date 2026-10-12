#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../infrastructure/service/platform.h"

// Specify the MCU
#define CFG_TUSB_MCU (OPT_MCU_STM32F4)
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

// Enable Dehug
#define CFG_TUSB_DEBUG (0)

// Enable OS
#define CFG_TUSB_OS (OPT_OS_FREERTOS)

// Enable device stack
#define CFG_TUD_ENABLED (1)

// Enable device class
#define CFG_TUD_CDC (1)
#define CFG_TUD_MSC (0)
#define CFG_TUD_HID (0)
#define CFG_TUD_MIDI (0)
#define CFG_TUD_VENDOR (0)

// Enable use of notification endpoint
#define CFG_TUD_CDC_NOTIFY (1)

#define BOARD_TUD_RHPORT (0)

#if TU_CHECK_MCU(OPT_MCU_ESP32S2, OPT_MCU_ESP32S3)
#define CFG_TUSB_OS_INC_PATH freertos /
#endif

#define BOARD_TUD_MAX_SPEED (OPT_MODE_DEFAULT_SPEED)

#define CFG_TUD_ENDPOINT0_SIZE (64u)

// CDC FIFO size of TX and RX
#define CFG_TUD_CDC_RX_BUFSIZE (512u)
#define CFG_TUD_CDC_TX_BUFSIZE (512u)

// Vendor FIFO size of TX and RX
// If zero: vendor endpoints will not be buffered
#define CFG_TUD_VENDOR_RX_BUFSIZE (512u)
#define CFG_TUD_VENDOR_TX_BUFSIZE (512u)

#define pdTICKS_TO_MS(xTimeInMs) ((TickType_t)((xTimeInMs) / portTICK_PERIOD_MS))

#ifdef __cplusplus
}
#endif

#endif
