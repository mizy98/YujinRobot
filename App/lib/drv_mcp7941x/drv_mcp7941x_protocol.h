#ifndef DRV_MCP7941X_PROTOCOL_H
#define DRV_MCP7941X_PROTOCOL_H

//! DO NOT include this header file directly. Include drv_mcp7941x.h instead.

#include "drv_mcp7941x_define.h"

#define DRV_MCP7941X_OK (0)
#define DRV_MCP7941X_FAIL (-1)

#define DRV_MCP7941X_SRAM_SIZE (64u)
#define DRV_MCP7941X_SRAM_PAGE_SIZE (1u)
#define DRV_MCP7941X_SRAM_START_ADDRESS (0x20)
#define DRV_MCP7941X_SRAM_END_ADDRESS \
    (DRV_MCP7941X_SRAM_START_ADDRESS + DRV_MCP7941X_SRAM_SIZE - 1)  // 0x20 - 0x5F

#define DRV_MCP7941X_EEPROM_SIZE (1024u)
#define DRV_MCP7941X_EEPROM_PAGE_SIZE (8u)
#define DRV_MCP7941X_EEPROM_START_ADDRESS (0x00)
#define DRV_MCP7941X_EEPROM_END_ADDRESS  \
    (DRV_MCP7941X_EEPROM_START_ADDRESS + \
     (DRV_MCP7941X_EEPROM_SIZE / DRV_MCP7941X_EEPROM_PAGE_SIZE) - 1)  // 0x00 - 0x7F

#define DRV_MCP7941X_TIMEOUT_COUNT (1000u)

#define self (drv_mcp7941x_private)

typedef enum {
    /* Timekeeping */
    RTCSEC = 0x00,
    RTCMIN = 0x01,
    RTCHOUR = 0x02,
    RTCWKDAY = 0x03,
    RTCDATE = 0x04,
    RTCMTH = 0x05,
    RTCYEAR = 0x06,
    CONTROL = 0x07,
    OSCTRIM = 0x08,
    EEUNLOCK = 0x09,

    /* Alarm 0 */
    ALM0SEC = 0x0A,
    ALM0MIN = 0x0B,
    ALM0HOUR = 0x0C,
    ALM0WKDAY = 0x0D,
    ALM0DATE = 0x0E,
    ALM0MTH = 0x0F,

    /* Alarm 1 */
    ALM1SEC = 0x11,
    ALM1MIN = 0x12,
    ALM1HOUR = 0x13,
    ALM1WKDAY = 0x14,
    ALM1DATE = 0x15,
    ALM1MTH = 0x16,

    /* Power-Down Time Stamp */
    PWRDNMIN = 0x18,
    PWRDNHOUR = 0x19,
    PWRDNDATE = 0x1A,
    PWRDNMTH = 0x1B,

    /* Power-Up Time Stamp */
    PWRUPMIN = 0x1C,
    PWRUPHOUR = 0x1D,
    PWRUPDATE = 0x1E,
    PWRUPMTH = 0x1F,
} RtccRegisterAddress;

typedef union {
    struct {
        uint8_t secone0 : 1;
        uint8_t secone1 : 1;
        uint8_t secone2 : 1;
        uint8_t secone3 : 1;
        uint8_t secten0 : 1;
        uint8_t secten1 : 1;
        uint8_t secten2 : 1;
        uint8_t st : 1;
    };
    uint8_t value;
} Rtcsec;

typedef union {
    struct {
        uint8_t minone0 : 1;
        uint8_t minone1 : 1;
        uint8_t minone2 : 1;
        uint8_t minone3 : 1;
        uint8_t minten0 : 1;
        uint8_t minten1 : 1;
        uint8_t minten2 : 1;
        uint8_t reserved : 1;
    };
    uint8_t value;
} Rtcmin;

typedef union {
    struct {
        uint8_t hourone0 : 1;
        uint8_t hourone1 : 1;
        uint8_t hourone2 : 1;
        uint8_t hourone3 : 1;
        uint8_t hourten0 : 1;
        uint8_t am_pm : 1;
        uint8_t hr24_12 : 1;
        uint8_t reserved : 1;
    };
    uint8_t value;
} Rtchour;

typedef union {
    struct {
        uint8_t wkday0 : 1;
        uint8_t wkday1 : 1;
        uint8_t wkday2 : 1;
        uint8_t vbaten : 1;
        uint8_t pwrfail : 1;
        uint8_t oscrun : 1;
        uint8_t reserved : 2;
    };
    uint8_t value;
} Rtcwkday;

typedef union {
    struct {
        uint8_t dateone0 : 1;
        uint8_t dateone1 : 1;
        uint8_t dateone2 : 1;
        uint8_t dateone3 : 1;
        uint8_t dateten0 : 1;
        uint8_t dateten1 : 1;
        uint8_t reserved : 2;
    };
    uint8_t value;
} Rtcdate;

typedef union {
    struct {
        uint8_t mthone0 : 1;
        uint8_t mthone1 : 1;
        uint8_t mthone2 : 1;
        uint8_t mthone3 : 1;
        uint8_t mthten0 : 1;
        uint8_t lpyr : 1;
        uint8_t reserved : 2;
    };
    uint8_t value;
} Rtcmth;

typedef union {
    struct {
        uint8_t yearone0 : 1;
        uint8_t yearone1 : 1;
        uint8_t yearone2 : 1;
        uint8_t yearone3 : 1;
        uint8_t yearten0 : 1;
        uint8_t yearten1 : 1;
        uint8_t yearten2 : 1;
        uint8_t yearten3 : 1;
    };
    uint8_t value;
} Rtcyear;

typedef union {
    struct {
        uint8_t sqwfs0 : 1;
        uint8_t sqwfs1 : 1;
        uint8_t crstrim : 1;
        uint8_t extosc : 1;
        uint8_t alm0en : 1;
        uint8_t alm1en : 1;
        uint8_t sqwen : 1;
        uint8_t out : 1;
    };
    uint8_t value;
} Control;

typedef struct {
    DrvMcp7941xInterface interface;
} DrvMcp7941Private;

extern DrvMcp7941Private drv_mcp7941x_private;

uint8_t DrvMcp7941x_ToBcd(uint8_t value);
uint8_t DrvMcp7941x_FromBcd(uint8_t bcd);
int DrvMcp7941x_Write(uint8_t address, uint8_t data);
int DrvMcp7941x_Read(uint8_t address, uint8_t* data);
int DrvMcp7941x_BurstWrite(uint8_t start_address, const uint8_t* data, uint16_t length);
int DrvMcp7941x_BurstRead(uint8_t start_address, uint8_t* data, uint16_t length);
int DrvMcp7941x_EepromWrite(uint8_t address, const uint8_t* data, const uint16_t length);
int DrvMcp7941x_EepromRead(uint8_t address, uint8_t* data, const uint16_t length);
int DrvMcp7941x_Start();
int DrvMcp7941x_Stop();
#endif