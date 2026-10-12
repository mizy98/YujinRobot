#ifndef DRV_MCP7941X_H
#define DRV_MCP7941X_H

#include "drv_mcp7941x_define.h"

int DrvMcp7941x_Initialize(DrvMcp7941xInterface* interface, bool* is_reset);
int DrvMcp7941x_DeInitialize(void);
int DrvMcp7941x_SetTime(DrvMcp7941xDateTime* date_time);
int DrvMcp7941x_GetTime(DrvMcp7941xDateTime* date_time);
int DrvMcp7941x_WriteSram(uint8_t address, const uint8_t* data, const uint16_t length);
int DrvMcp7941x_ReadSram(uint8_t address, uint8_t* data, const uint16_t length);
int DrvMcp7941x_WriteEeprom(uint8_t address, const uint8_t* data, const uint16_t length);
int DrvMcp7941x_ReadEeprom(uint8_t address, uint8_t* data, const uint16_t length);
int DrvMcp7941x_GetStatus(DrvMcp7941xStatus* status);
void DrvMcp7941x_GetSramInfo(DrvMcp7941xSramInfo* info);
void DrvMcp7941x_GetEepromInfo(DrvMcp7941xEepromInfo* info);

#endif  // DRV_MCP7941X_H
