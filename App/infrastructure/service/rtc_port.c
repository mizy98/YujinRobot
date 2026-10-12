#include "rtc_port.h"

#include <time.h>

#include "../../lib/drv_mcp7941x/drv_mcp7941x.h"
#include "../service/i2c_port.h"
#include "../service/platform.h"

#define self (rtc_port_private)

#define UNIX_OFFSET_YEAR (1970u)
#define POSIX_OFFSET_YEAR (1900u)

#define DATA_MAGIC (0x424B5551)  // 'BKUP' in ASCII

static struct {
    bool is_initialized;
    bool is_backup_format_occurred;

    DrvMcp7941xSramInfo drv_sram_info;
    uint8_t drv_sram_max_index;

    AppErrorInfo error_info;
} rtc_port_private;

static uint64_t _toUnixTimestamp(struct tm* time_info) {
    int year = time_info->tm_year + POSIX_OFFSET_YEAR;
    int month = time_info->tm_mon;

    static const uint16_t days_before_month[] = {0,   31,  59,  90,  120, 151,
                                                 181, 212, 243, 273, 304, 334};

    uint64_t days = 0;

    // year difference
    int y = year - UNIX_OFFSET_YEAR;

    days = y * 365 + (y + 1) / 4 - (y + 69) / 100 + (y + 369) / 400;

    // month difference
    days += days_before_month[month];

    // leap year correction
    if (month > 1 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
        days += 1;
    }

    days += time_info->tm_mday - 1;

    return days * 86400ULL + time_info->tm_hour * 3600ULL + time_info->tm_min * 60ULL +
           time_info->tm_sec;
}

static void _toPosixTimeInfo(uint64_t timestamp, struct tm* time_info) {
    uint32_t sec = timestamp % 60;
    timestamp /= 60;
    uint32_t min = timestamp % 60;
    timestamp /= 60;
    uint32_t hour = timestamp % 24;
    timestamp /= 24;

    // days since 1970-01-01
    uint64_t days = timestamp;

    int year = 1970;

    uint64_t n400 = days / 146097;
    year += n400 * 400;
    days -= n400 * 146097;

    uint64_t n100 = days / 36524;
    if (n100 == 4) n100 = 3;
    year += n100 * 100;
    days -= n100 * 36524;

    uint64_t n4 = days / 1461;
    year += n4 * 4;
    days -= n4 * 1461;

    uint64_t n1 = days / 365;
    if (n1 == 4) n1 = 3;
    year += n1;
    days -= n1 * 365;

    int leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));

    static const uint8_t dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int month = 0;
    while (1) {
        int d = dim[month];
        if (month == 1 && leap) d++;

        if (days < d) break;
        days -= d;
        month++;
    }

    time_info->tm_year = year - 1900;
    time_info->tm_mon = month;
    time_info->tm_mday = days + 1;

    time_info->tm_hour = hour;
    time_info->tm_min = min;
    time_info->tm_sec = sec;

    time_info->tm_wday = (timestamp + 4) % 7;  // 1970-01-01 = Thursday (4)
    time_info->tm_yday = 0;
    time_info->tm_isdst = 0;
}

static int _write(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                  const uint16_t length) {
    return I2cPort_Write(device_address << 1, register_address, data, length);
}

static int _read(uint8_t device_address, uint8_t register_address, uint8_t* data,
                 const uint16_t length) {
    return I2cPort_Read(device_address << 1, register_address, data, length);
}

AppResult RtcPort_Initialize(void) {
    if (!I2cPort_IsInitialized()) {
        if (I2cPort_Initialize() != APP_RESULT_OK) {
            I2cPort_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    bool is_reset = false;

    if (DrvMcp7941x_Initialize(
            &(DrvMcp7941xInterface){
                .write = _write,
                .read = _read,
                .delayMs = (void (*)(uint32_t))osDelay,
            },
            &is_reset) != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_RTC_PORT_DRV_INIT);
        return APP_RESULT_FAIL;
    }

    self.is_backup_format_occurred = is_reset;

    DrvMcp7941xSramInfo sram_info = {0};
    DrvMcp7941x_GetSramInfo(&sram_info);

    self.drv_sram_info = sram_info;
    self.drv_sram_max_index = sram_info.size / sizeof(uint32_t);

    self.error_info = (AppErrorInfo){0};
    self.is_initialized = true;

    return APP_RESULT_OK;
}

AppResult RtcPort_Deinitialize(void) {
    int result = DrvMcp7941x_DeInitialize();
    if (result != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_RTC_PORT_DRV_DEINIT);
        return APP_RESULT_FAIL;
    }

    if (I2cPort_IsInitialized()) {
        if (I2cPort_Deinitialize() != APP_RESULT_OK) {
            I2cPort_GetLastErrorInfo(&self.error_info);
            return APP_RESULT_FAIL;
        }
    }

    self.is_initialized = false;

    return APP_RESULT_OK;
}

bool RtcPort_IsInitialized(void) { return self.is_initialized; }

void RtcPort_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

AppResult RtcPort_SetTimestamp(uint64_t timestamp) {
    struct tm time_info;
    _toPosixTimeInfo(timestamp, &time_info);

    DrvMcp7941xDateTime date_time = {0};
    date_time.year = time_info.tm_year - (RTC_OFFSET_YEAR - POSIX_OFFSET_YEAR);
    date_time.month = time_info.tm_mon + 1;
    date_time.date = time_info.tm_mday;
    date_time.hour = time_info.tm_hour;
    date_time.minute = time_info.tm_min;
    date_time.second = time_info.tm_sec;

    if (DrvMcp7941x_SetTime(&date_time) != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_RTC_PORT_DRV_SET_TIME);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult RtcPort_GetTimestamp(uint64_t* timestamp) {
    DrvMcp7941xDateTime date_time = {0};
    if (DrvMcp7941x_GetTime(&date_time) != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_RTC_PORT_DRV_GET_TIME);
        return APP_RESULT_FAIL;
    }

    struct tm time_info = {0};
    time_info.tm_year = date_time.year + (RTC_OFFSET_YEAR - POSIX_OFFSET_YEAR);
    time_info.tm_mon = date_time.month - 1;
    time_info.tm_mday = date_time.date;
    time_info.tm_hour = date_time.hour;
    time_info.tm_min = date_time.minute;
    time_info.tm_sec = date_time.second;

    *timestamp = _toUnixTimestamp(&time_info);

    return APP_RESULT_OK;
}

AppResult RtcPort_ReadBackupRegister(uint8_t index, uint32_t* data) {
    if (index >= self.drv_sram_max_index) {
        return APP_RESULT_FAIL;
    }

    if (DrvMcp7941x_ReadSram(index * sizeof(uint32_t), (uint8_t*)data, sizeof(uint32_t)) != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_RTC_PORT_DRV_READ_SRAM);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult RtcPort_WriteBackupRegister(uint8_t index, uint32_t data) {
    if (index >= self.drv_sram_max_index) {
        return APP_RESULT_FAIL;
    }

    if (DrvMcp7941x_WriteSram(index * sizeof(uint32_t), (uint8_t*)&data, sizeof(uint32_t)) != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_RTC_PORT_DRV_WRITE_SRAM);
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

void RtcPort_GetBackupRegisterInfo(uint8_t* register_count, uint8_t* register_size) {
    if (register_count != NULL) {
        *register_count = self.drv_sram_max_index;
    }
    if (register_size != NULL) {
        *register_size = sizeof(uint32_t);
    }
}

bool RtcPort_IsBackupFormatOccurred(void) { return self.is_backup_format_occurred; }
