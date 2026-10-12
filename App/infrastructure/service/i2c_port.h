#ifndef I2C_PORT_H
#define I2C_PORT_H

#include <stdint.h>

#include "../../domain/model/app_model.h"
#include "platform.h"

AppResult I2cPort_Initialize(void);
AppResult I2cPort_Deinitialize(void);
void I2cPort_GetLastErrorInfo(AppErrorInfo* info);
bool I2cPort_IsInitialized(void);
int I2cPort_Write(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                  uint16_t length);
int I2cPort_Read(uint8_t device_address, uint8_t register_address, uint8_t* data, uint16_t length);

#endif
