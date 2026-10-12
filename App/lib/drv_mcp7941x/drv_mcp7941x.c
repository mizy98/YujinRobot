#include "drv_mcp7941x.h"

#include "drv_mcp7941x_protocol.h"

int DrvMcp7941x_Initialize(DrvMcp7941xInterface* interface, bool* is_reset) {
    if (interface == NULL || interface->write == NULL || interface->read == NULL ||
        interface->delayMs == NULL || is_reset == NULL) {
        return DRV_MCP7941X_FAIL;
    }

    self.interface = *interface;

    Control control = {0};
    Rtcsec rtcsec = {0};
    Rtcwkday rtcwkday = {0};

    if (DrvMcp7941x_Read(RTCWKDAY, &rtcwkday.value) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    if (rtcwkday.oscrun == 0) {
        control.extosc = 0;
        if (DrvMcp7941x_Write(CONTROL, control.value) != 0) {
            return DRV_MCP7941X_FAIL;
        }

        rtcwkday.vbaten = 1;
        if (DrvMcp7941x_Write(RTCWKDAY, rtcwkday.value) != 0) {
            return DRV_MCP7941X_FAIL;
        }

        rtcsec.st = 1;
        if (DrvMcp7941x_Write(RTCSEC, rtcsec.value) != 0) {
            return DRV_MCP7941X_FAIL;
        }

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

        *is_reset = true;

        uint8_t sram_data = 0xFF;
        if (DrvMcp7941x_WriteSram(0, &sram_data, DRV_MCP7941X_SRAM_SIZE) != 0) {
            return DRV_MCP7941X_FAIL;
        }
    } else {
        *is_reset = false;
    }

    return DRV_MCP7941X_OK;
}

int DrvMcp7941x_DeInitialize(void) { return DRV_MCP7941X_OK; }

int DrvMcp7941x_SetTime(DrvMcp7941xDateTime* date_time) {
    if (DrvMcp7941x_Stop() != DRV_MCP7941X_OK) {
        return DRV_MCP7941X_FAIL;
    }

    uint8_t buffer[7] = {0};

    Rtcsec* rtcsec = (Rtcsec*)&buffer[0];
    Rtcmin* rtcmin = (Rtcmin*)&buffer[1];
    Rtchour* rtchour = (Rtchour*)&buffer[2];
    Rtcwkday* rtcwkday = (Rtcwkday*)&buffer[3];
    Rtcdate* rtcdate = (Rtcdate*)&buffer[4];
    Rtcmth* rtcmth = (Rtcmth*)&buffer[5];
    Rtcyear* rtcyear = (Rtcyear*)&buffer[6];

    uint8_t second_bcd = DrvMcp7941x_ToBcd(date_time->second);
    uint8_t minute_bcd = DrvMcp7941x_ToBcd(date_time->minute);
    uint8_t hour_bcd = DrvMcp7941x_ToBcd(date_time->hour);
    uint8_t date_bcd = DrvMcp7941x_ToBcd(date_time->date);
    uint8_t month_bcd = DrvMcp7941x_ToBcd(date_time->month);
    uint8_t year_bcd = DrvMcp7941x_ToBcd(date_time->year);

    rtcsec->value = second_bcd;
    rtcmin->value = minute_bcd;
    rtchour->value = hour_bcd;
    rtcdate->value = date_bcd;
    rtcmth->value = month_bcd;
    rtcyear->value = year_bcd;

    rtcsec->st = 1;  // Start the oscillator
    rtcwkday->vbaten = 1;

    if (DrvMcp7941x_BurstWrite(RTCSEC, buffer, sizeof(buffer)) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    return DRV_MCP7941X_OK;
}

int DrvMcp7941x_GetTime(DrvMcp7941xDateTime* date_time) {
    uint8_t buffer[7] = {0};

    if (DrvMcp7941x_BurstRead(RTCSEC, buffer, sizeof(buffer)) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    Rtcsec* rtcsec = (Rtcsec*)&buffer[0];
    Rtcmin* rtcmin = (Rtcmin*)&buffer[1];
    Rtchour* rtchour = (Rtchour*)&buffer[2];
    Rtcwkday* rtcwkday = (Rtcwkday*)&buffer[3];
    Rtcdate* rtcdate = (Rtcdate*)&buffer[4];
    Rtcmth* rtcmth = (Rtcmth*)&buffer[5];
    Rtcyear* rtcyear = (Rtcyear*)&buffer[6];

    date_time->second = DrvMcp7941x_FromBcd(rtcsec->value & 0x7F);
    date_time->minute = DrvMcp7941x_FromBcd(rtcmin->value & 0x7F);
    date_time->hour = DrvMcp7941x_FromBcd(rtchour->value & 0x3F);
    date_time->day_of_week = rtcwkday->value & 0x07;
    date_time->date = DrvMcp7941x_FromBcd(rtcdate->value & 0x3F);
    date_time->month = DrvMcp7941x_FromBcd(rtcmth->value & 0x1F);
    date_time->year = DrvMcp7941x_FromBcd(rtcyear->value);

    return DRV_MCP7941X_OK;
}

int DrvMcp7941x_WriteSram(uint8_t address, const uint8_t* data, const uint16_t length) {
    if (address >= DRV_MCP7941X_SRAM_SIZE) {
        return DRV_MCP7941X_FAIL;
    }

    if (DrvMcp7941x_BurstWrite(DRV_MCP7941X_SRAM_START_ADDRESS + address, data, length) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    return DRV_MCP7941X_OK;
}

int DrvMcp7941x_ReadSram(uint8_t address, uint8_t* data, const uint16_t length) {
    if (address >= DRV_MCP7941X_SRAM_SIZE) {
        return DRV_MCP7941X_FAIL;
    }

    if (DrvMcp7941x_BurstRead(DRV_MCP7941X_SRAM_START_ADDRESS + address, data, length) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    return DRV_MCP7941X_OK;
}

int DrvMcp7941x_WriteEeprom(uint8_t address, const uint8_t* data, const uint16_t length) {
    if (address >= (DRV_MCP7941X_EEPROM_SIZE / DRV_MCP7941X_EEPROM_PAGE_SIZE)) {
        return DRV_MCP7941X_FAIL;
    }

    if (length > DRV_MCP7941X_EEPROM_PAGE_SIZE) {
        return DRV_MCP7941X_FAIL;
    }

    if (DrvMcp7941x_EepromWrite(address, data, length) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    return DRV_MCP7941X_OK;
}

int DrvMcp7941x_ReadEeprom(uint8_t address, uint8_t* data, const uint16_t length) {
    if (address >= (DRV_MCP7941X_EEPROM_SIZE / DRV_MCP7941X_EEPROM_PAGE_SIZE)) {
        return DRV_MCP7941X_FAIL;
    }

    if (DrvMcp7941x_EepromRead(
            address, data,
            length > DRV_MCP7941X_EEPROM_PAGE_SIZE ? DRV_MCP7941X_EEPROM_PAGE_SIZE : length) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    return DRV_MCP7941X_OK;
}

int DrvMcp7941x_GetStatus(DrvMcp7941xStatus* status) {
    Rtcwkday rtcwkday = {0};

    if (DrvMcp7941x_Read(RTCWKDAY, (uint8_t*)&rtcwkday) != 0) {
        return DRV_MCP7941X_FAIL;
    }

    status->is_oscillator_running = rtcwkday.oscrun;
    status->power_fail_detected = rtcwkday.pwrfail;

    return DRV_MCP7941X_OK;
}

void DrvMcp7941x_GetSramInfo(DrvMcp7941xSramInfo* info) {
    if (info != NULL) {
        info->size = DRV_MCP7941X_SRAM_SIZE;
        info->page_size = DRV_MCP7941X_SRAM_PAGE_SIZE;
    }
}

void DrvMcp7941x_GetEepromInfo(DrvMcp7941xEepromInfo* info) {
    if (info != NULL) {
        info->size = DRV_MCP7941X_EEPROM_SIZE;
        info->page_size = DRV_MCP7941X_EEPROM_PAGE_SIZE;
    }
}
