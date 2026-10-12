#include "drv_mt29fx_protocol.h"

static DrvMt29FxInterface drv_mt29fx_interface;
static uint8_t max_plane_count;

void DrvMt29Fx_SetInterface(DrvMt29FxInterface* interface) { drv_mt29fx_interface = *interface; }

void DrvMt29Fx_RegisterPlaneInfo(uint8_t max_count) { max_plane_count = max_count; }

void DrvMt29Fx_SetWriteProtection(uint8_t state) {
    if (state == 0) {
        drv_mt29fx_interface.setWpPin(1);
    } else {
        drv_mt29fx_interface.setWpPin(0);
    }
}

int DrvMt29Fx_WriteCommand(const uint8_t command, const uint8_t* address_bytes,
                           const uint8_t address_byte_length, const uint8_t* data,
                           const uint16_t data_length) {
    drv_mt29fx_interface.setCsPin(0);

    int result = drv_mt29fx_interface.write((uint8_t*)&command, 1);
    if (result != 0) {
        drv_mt29fx_interface.setCsPin(1);
        return result;
    }

    if (address_bytes != NULL && address_byte_length > 0) {
        result = drv_mt29fx_interface.write((uint8_t*)address_bytes, address_byte_length);
        if (result != 0) {
            drv_mt29fx_interface.setCsPin(1);
            return result;
        }
    }

    if (data != NULL && data_length > 0) {
        result = drv_mt29fx_interface.write((uint8_t*)data, data_length);
        if (result != 0) {
            drv_mt29fx_interface.setCsPin(1);
            return result;
        }
    }

    drv_mt29fx_interface.setCsPin(1);
    return 0;
}

int DrvMt29Fx_ReadCommand(const uint8_t command, const uint8_t* address_bytes,
                          const uint8_t address_byte_length, uint8_t* data,
                          const uint16_t data_length) {
    drv_mt29fx_interface.setCsPin(0);

    int result = drv_mt29fx_interface.write((uint8_t*)&command, 1);
    if (result != 0) {
        drv_mt29fx_interface.setCsPin(1);
        return result;
    }

    if (address_bytes != NULL && address_byte_length > 0) {
        result = drv_mt29fx_interface.write((uint8_t*)address_bytes, address_byte_length);
        if (result != 0) {
            drv_mt29fx_interface.setCsPin(1);
            return result;
        }
    }

    if (data != NULL && data_length > 0) {
        result = drv_mt29fx_interface.read((uint8_t*)data, data_length);
        if (result != 0) {
            drv_mt29fx_interface.setCsPin(1);
            return result;
        }
    }

    drv_mt29fx_interface.setCsPin(1);
    return 0;
}

int DrvMt29Fx_WaitForNotInOperation() {
    uint32_t timeout = TIMEOUT_TRY_COUNT;
    StatusRegister status;

    while (--timeout) {
        if (DrvMt29Fx_ReadStatus(&status) != 0) {
            return -1;
        }

        if (status.oip == 0) {
            return 0;
        }
    }

    return -1;
}

bool DrvMt29Fx_IsProgrammed() {
    StatusRegister status;

    (void)DrvMt29Fx_ReadStatus(&status);

    if (status.p_fail == 0) {
        return true;
    }

    return false;
}

bool DrvMt29Fx_IsErased() {
    StatusRegister status;

    (void)DrvMt29Fx_ReadStatus(&status);

    if (status.e_fail == 0) {
        return true;
    }

    return false;
}

int DrvMt29Fx_ReadStatus(StatusRegister* status) {
    const uint8_t address_byte = STATUS;

    return DrvMt29Fx_ReadCommand(GET_FEATURES, &address_byte, sizeof(address_byte),
                                 (uint8_t*)status, sizeof(StatusRegister));
}

int DrvMt29Fx_ReadBlockLockStatus(BlockLockRegister* block_lock) {
    const uint8_t address_byte = BLOCK_LOCK;

    return DrvMt29Fx_ReadCommand(GET_FEATURES, &address_byte, sizeof(address_byte),
                                 (uint8_t*)block_lock, sizeof(BlockLockRegister));
}

int DrvMt29Fx_ReadConfiguration(ConfigurationRegister* configuration) {
    const uint8_t address_byte = CONFIGURATION;

    return DrvMt29Fx_ReadCommand(GET_FEATURES, &address_byte, sizeof(address_byte),
                                 (uint8_t*)configuration, sizeof(ConfigurationRegister));
}

int DrvMt29Fx_ReadDieSelect(DieSelectRegister* die_select) {
    const uint8_t address_byte = DIE_SELECT;

    return DrvMt29Fx_ReadCommand(GET_FEATURES, &address_byte, sizeof(address_byte),
                                 (uint8_t*)die_select, sizeof(DieSelectRegister));
}

int DrvMt29Fx_ReadDeviceId(uint8_t* manufacturer_id, uint8_t* device_id) {
    uint8_t dummy = 0x00;
    uint8_t datas[2] = {0};

    int result = DrvMt29Fx_ReadCommand(READ_ID, &dummy, sizeof(dummy), datas, sizeof(datas));
    if (result != 0) {
        return result;
    }

    *manufacturer_id = datas[0];
    *device_id = datas[1];

    return 0;
}

int DrvMt29Fx_ReadParameter(Parameter* parameter, const ConfigurationRegister* configuration) {
    const uint8_t address_byte = CONFIGURATION;
    ConfigurationRegister config = {
        // Access OTP area/Parameter/Unique ID
        .cfg0 = 0,
        .cfg1 = 1,
        .cfg2 = 0,
        .lot_en = configuration->lot_en,
        .ecc_en = configuration->ecc_en,
    };
    DrvMt29FxAddress address = {
        .block = 0,
        .page = 1,
    };
    uint8_t cache_buffer[sizeof(Parameter)];

    int result = DrvMt29Fx_WriteCommand(SET_FEATURES, &address_byte, sizeof(address_byte),
                                        (uint8_t*)&config, sizeof(config));
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_TransferToCache(&address);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WaitForNotInOperation();
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_ReadFromCache(&address, cache_buffer, sizeof(Parameter));
    if (result != 0) {
        return result;
    }

    config.cfg0 = 0;
    config.cfg1 = 0;
    config.cfg2 = 0;

    result = DrvMt29Fx_WriteCommand(SET_FEATURES, &address_byte, sizeof(address_byte),
                                    (uint8_t*)&config, sizeof(config));
    if (result != 0) {
        return result;
    }

    memcpy(parameter, cache_buffer, sizeof(Parameter));

    return 0;
}

int DrvMt29Fx_WriteConfiguration(const ConfigurationRegister* configuration) {
    const uint8_t address_byte = CONFIGURATION;

    return DrvMt29Fx_WriteCommand(SET_FEATURES, &address_byte, sizeof(address_byte),
                                  (uint8_t*)configuration, sizeof(ConfigurationRegister));
}

int DrvMt29Fx_EnableWrite() {
    int result = 0;
    uint32_t timeout = TIMEOUT_TRY_COUNT;

    result = DrvMt29Fx_WriteCommand(WRITE_ENABLE, NULL, 0, NULL, 0);
    if (result != 0) {
        return result;
    }

    StatusRegister status = {0};

    while (--timeout) {
        result = DrvMt29Fx_ReadStatus(&status);
        if (result != 0) {
            return result;
        }

        if (status.wel == 1) {
            return 0;
        }
    }

    return -1;
}

int DrvMt29Fx_DisableWrite() {
    int result = 0;
    uint32_t timeout = TIMEOUT_TRY_COUNT;

    result = DrvMt29Fx_WriteCommand(WRITE_DISABLE, NULL, 0, NULL, 0);
    if (result != 0) {
        return result;
    }

    StatusRegister status = {0};

    while (--timeout) {
        result = DrvMt29Fx_ReadStatus(&status);
        if (result != 0) {
            return result;
        }

        if (status.wel == 0) {
            return 0;
        }
    }

    return -1;
}

int DrvMt29Fx_TransferToCache(const DrvMt29FxAddress* address) {
    uint8_t command = PAGE_READ;
    uint32_t row = DrvMt29Fx_ToRowAddress(address);
    uint8_t buffer[3] = {
        (uint8_t)(row >> 16),
        (uint8_t)(row >> 8),
        (uint8_t)(row & 0xFF),
    };

    return DrvMt29Fx_WriteCommand(command, buffer, sizeof(buffer), NULL, 0);
}

int DrvMt29Fx_TransferToMain(const DrvMt29FxAddress* address) {
    uint8_t command = PROGRAM_EXECUTE;
    uint32_t row = DrvMt29Fx_ToRowAddress(address);
    uint8_t buffer[3] = {
        (uint8_t)(row >> 16),
        (uint8_t)(row >> 8),
        (uint8_t)(row & 0xFF),
    };

    return DrvMt29Fx_WriteCommand(command, buffer, sizeof(buffer), NULL, 0);
}

int DrvMt29Fx_WriteToCache(const DrvMt29FxAddress* address, const uint8_t* data,
                           const uint16_t length) {
    const uint8_t command = PROGRAM_LOAD_X1;
    const uint32_t column =
        max_plane_count > 1 ? DrvMt29Fx_ToMultiPlaneColumnAddress(address) : address->column;
    uint8_t buffer[2] = {
        (uint8_t)(column >> 8),
        (uint8_t)(column & 0xFF),
    };

    return DrvMt29Fx_WriteCommand(command, buffer, sizeof(buffer), data, length);
}

int DrvMt29Fx_ReadFromCache(const DrvMt29FxAddress* address, uint8_t* data, const uint16_t length) {
    const uint8_t command = READ_FROM_CACHE_X1;
    const uint32_t column =
        max_plane_count > 1 ? DrvMt29Fx_ToMultiPlaneColumnAddress(address) : address->column;
    uint8_t buffer[3] = {
        (uint8_t)(column >> 8),
        (uint8_t)(column & 0xFF),
    };

    return DrvMt29Fx_ReadCommand(command, buffer, sizeof(buffer), data, length);
}

int DrvMt29Fx_EraseBlock(const DrvMt29FxAddress* address) {
    const uint8_t command = BLOCK_ERASE;
    const uint32_t row = DrvMt29Fx_ToRowAddress(address);
    uint8_t buffer[3] = {
        (uint8_t)(row >> 16),
        (uint8_t)(row >> 8),
        (uint8_t)(row & 0xFF),
    };

    return DrvMt29Fx_WriteCommand(command, buffer, sizeof(buffer), NULL, 0);
}

uint32_t DrvMt29Fx_ToRowAddress(const DrvMt29FxAddress* address) {
    return (address->block << ROW_ADDRESS_BLOCK_MASK) | address->page;
}

uint32_t DrvMt29Fx_ToMultiPlaneColumnAddress(const DrvMt29FxAddress* address) {
    uint8_t plane_address = address->block % max_plane_count;

    return (plane_address << COLUMN_ADDRESS_PLANE_MASK) | address->column;
}