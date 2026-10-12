#include "drv_mcp7941x_protocol.h"

#define I2C_ADDRESS_RTCC_SRAM (0b1101111)
#define I2C_ADDRESS_EEPROM (0b1010111)

DrvMcp7941Private drv_mcp7941x_private;

uint8_t DrvMcp7941x_ToBcd(uint8_t value) {
    uint8_t tens = value / 10;
    uint8_t ones = value % 10;
    return (tens << 4) | ones;
}

uint8_t DrvMcp7941x_FromBcd(uint8_t bcd) {
    uint8_t tens = (bcd >> 4) & 0x0F;
    uint8_t ones = bcd & 0x0F;
    return tens * 10 + ones;
}

int DrvMcp7941x_Write(uint8_t address, uint8_t data) {
    return self.interface.write(I2C_ADDRESS_RTCC_SRAM, address, &data, 1);
}

int DrvMcp7941x_Read(uint8_t address, uint8_t* data) {
    return self.interface.read(I2C_ADDRESS_RTCC_SRAM, address, data, 1);
}

int DrvMcp7941x_BurstWrite(uint8_t start_address, const uint8_t* data, uint16_t length) {
    return self.interface.write(I2C_ADDRESS_RTCC_SRAM, start_address, data, length);
}

int DrvMcp7941x_BurstRead(uint8_t start_address, uint8_t* data, uint16_t length) {
    return self.interface.read(I2C_ADDRESS_RTCC_SRAM, start_address, data, length);
}

int DrvMcp7941x_EepromWrite(uint8_t address, const uint8_t* data, const uint16_t length) {
    return self.interface.write(I2C_ADDRESS_EEPROM, address, data, length);
}

int DrvMcp7941x_EepromRead(uint8_t address, uint8_t* data, const uint16_t length) {
    return self.interface.read(I2C_ADDRESS_EEPROM, address, data, length);
}

int DrvMcp7941x_Start() {
    Rtcsec rtcsec = {0};
    Rtcwkday rtcwkday = {0};

    if (DrvMcp7941x_Read(RTCSEC, &rtcsec.value) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    rtcsec.st = 1;

    if (DrvMcp7941x_Write(RTCSEC, rtcsec.value) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    // Wait for oscillator to start
    uint16_t timeout_count = DRV_MCP7941X_TIMEOUT_COUNT;
    do {
        self.interface.delayMs(1);
        if (DrvMcp7941x_Read(RTCWKDAY, &rtcwkday.value) != 0) {
            return DRV_MCP7941X_FAIL;
        }
        timeout_count--;
    } while (rtcwkday.oscrun == 0 && timeout_count > 0);

    if (timeout_count == 0) {
        return DRV_MCP7941X_FAIL;
    }

    return DRV_MCP7941X_OK;
}

int DrvMcp7941x_Stop() {
    Rtcsec rtcsec = {0};
    Rtcwkday rtcwkday = {0};

    if (DrvMcp7941x_Read(RTCSEC, &rtcsec.value) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    rtcsec.st = 0;

    if (DrvMcp7941x_Write(RTCSEC, rtcsec.value) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    // Wait for oscillator to stop
    uint16_t timeout_count = 1000;
    do {
        self.interface.delayMs(1);
        if (DrvMcp7941x_Read(RTCWKDAY, &rtcwkday.value) != 0) {
            return DRV_MCP7941X_FAIL;
        }
        timeout_count--;
    } while (rtcwkday.oscrun == 1 && timeout_count > 0);

    if (timeout_count == 0) {
        return DRV_MCP7941X_FAIL;
    }

    return DRV_MCP7941X_OK;
}
