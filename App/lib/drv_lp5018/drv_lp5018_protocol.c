#include "drv_lp5018_protocol.h"

int DrvLp5018_WriteRegister(uint8_t address, const uint8_t* data, const uint16_t length) {
    return self.interface.write(self.device_address, address, data, length);
}

int DrvLp5018_ReadRegister(uint8_t address, uint8_t* data, const uint16_t length) {
    return self.interface.read(self.device_address, address, data, length);
}
