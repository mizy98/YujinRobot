#include "drv_mt29fx_protocol.h"
#include "drv_mt29fx.h"

#define self drv_mt29fx_private

typedef struct {
    bool is_initialized;

    Model model;
    uint8_t manufacturer_id;
    uint8_t device_id;

    uint32_t write_error_times;
    uint32_t read_error_times;
    uint32_t erase_error_times;

    Parameter parameter;

    BlockLockRegister block_lock;
    ConfigurationRegister configuration;
    StatusRegister status;
    DieSelectRegister die_select;

    uint16_t bytes_per_page;
    uint16_t bytes_per_spare_page;
    uint16_t pages_per_block;
    uint16_t blocks_per_unit;
    uint16_t units_per_device;
    uint16_t planes_per_device;
    uint16_t bytes_per_partial_page;

    uint16_t spare_area_user_meta_data_1_offset_address;  // user meta data 1: ECC protected
    uint8_t spare_area_user_meta_data_1_size;
    uint16_t spare_area_user_meta_data_2_offset_address;  // user meta data 2: no ECC protected
    uint8_t spare_area_user_meta_data_2_size;

    uint16_t program_time;              // µs
    uint16_t erase_time;                // ms
    uint16_t read_time;                 // µs
    uint16_t change_column_setup_time;  // ns
    DrvMt29FxInterface interface;
} DrvMt29FxPrivate;

static const char mt29f1g01_name[] = {'M', 'T', '2', '9', 'F', '1', 'G', '0', '1'};
static const char mt29f2g01_name[] = {'M', 'T', '2', '9', 'F', '2', 'G', '0', '1'};
static const char mt29f4g01_name[] = {'M', 'T', '2', '9', 'F', '4', 'G', '0', '1'};
static const char mt29f8g01_name[] = {'M', 'T', '2', '9', 'F', '8', 'G', '0', '1'};

static DrvMt29FxPrivate drv_mt29fx_private;

static inline bool isValidAddress(const DrvMt29FxAddress* address) {
    if (address == NULL) {
        return false;
    }

    return (address->block < self.blocks_per_unit) && (address->page < self.pages_per_block);
}

static inline bool isValidPart(const DrvMt29FxColumnPart part) {
    return part >= DRV_MT29FX_COLUMN_PART_0 && part <= DRV_MT29FX_COLUMN_PART_3;
}

int DrvMt29Fx_Initialize(DrvMt29FxInterface* itf) {
    if (itf == NULL || itf->write == NULL || itf->read == NULL || itf->delayMs == NULL ||
        itf->setCsPin == NULL || itf->setWpPin == NULL || itf->setHoldPin == NULL) {
        return -1;
    }

    DrvMt29Fx_SetInterface(itf);

    memcpy(&self.interface, itf, sizeof(DrvMt29FxInterface));

    StatusRegister status;
    BlockLockRegister block_lock;
    ConfigurationRegister configuration;
    DieSelectRegister die_select = {0};
    Parameter parameter;
    uint8_t manufacturer_id, device_id;

    int result = DrvMt29Fx_Reset();
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_ReadStatus(&status);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_ReadBlockLockStatus(&block_lock);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_ReadDieSelect(&die_select);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_ReadDeviceId(&manufacturer_id, &device_id);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_ReadConfiguration(&configuration);
    if (result != 0) {
        return result;
    }

    configuration.ecc_en = 1;
    configuration.lot_en = 0;
    result = DrvMt29Fx_WriteConfiguration(&configuration);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_ReadParameter(&parameter, &configuration);
    if (result != 0) {
        return result;
    }

    // ONFI signature
    const uint8_t onfi_signature[] = {'O', 'N', 'F', 'I'};

    if (memcmp(parameter.parameter_signature, onfi_signature, 4) != 0) {
        return -1;
    }

    if (memcmp(parameter.device_model, mt29f1g01_name, sizeof(mt29f1g01_name)) == 0) {
        self.model = MODEL_MT29F1G01;
        self.planes_per_device = 1;

        // TODO: add spare area user meta data offset and size
    } else if (memcmp(parameter.device_model, mt29f2g01_name, sizeof(mt29f2g01_name)) == 0) {
        self.model = MODEL_MT29F2G01;
        self.planes_per_device = 2;

        self.spare_area_user_meta_data_1_offset_address = 0x820;
        self.spare_area_user_meta_data_1_size = 0x20;
        self.spare_area_user_meta_data_2_offset_address = 0x804;
        self.spare_area_user_meta_data_2_size = 0x0C;
    } else if (memcmp(parameter.device_model, mt29f4g01_name, sizeof(mt29f4g01_name)) == 0) {
        self.model = MODEL_MT29F4G01;
        self.planes_per_device = 1;

        // TODO: add spare area user meta data offset and size
    } else if (memcmp(parameter.device_model, mt29f8g01_name, sizeof(mt29f8g01_name)) == 0) {
        self.model = MODEL_MT29F8G01;
        self.planes_per_device = 2;

        // TODO: add spare area user meta data offset and size
    } else {
        self.model = MODEL_INVALID;
        return -1;
    }

    if (self.spare_area_user_meta_data_1_offset_address == 0 ||
        self.spare_area_user_meta_data_1_size == 0 ||
        self.spare_area_user_meta_data_2_offset_address == 0 ||
        self.spare_area_user_meta_data_2_size == 0) {
        return -1;
    }

    DrvMt29Fx_RegisterPlaneInfo(self.planes_per_device);

    DrvMt29Fx_SetWriteProtection(0);  // disable write protection

    self.bytes_per_page = parameter.num_data_bytes_per_page;
    self.bytes_per_spare_page = parameter.num_spare_bytes_per_page;
    self.pages_per_block = parameter.num_pages_per_block;
    self.blocks_per_unit = parameter.num_blocks_per_unit;
    self.units_per_device = parameter.num_logical_units;
    self.bytes_per_partial_page = parameter.num_data_bytes_per_partial_page;

    self.program_time = parameter.t_prog_max_page_program_time;
    self.erase_time = parameter.t_ers_max_block_erase_time;
    self.read_time = parameter.t_r_max_page_read_time;
    self.change_column_setup_time = parameter.t_ccs_min;

    memcpy(&self.block_lock, &block_lock, sizeof(BlockLockRegister));
    memcpy(&self.configuration, &configuration, sizeof(ConfigurationRegister));
    memcpy(&self.status, &status, sizeof(StatusRegister));
    memcpy(&self.die_select, &die_select, sizeof(DieSelectRegister));
    memcpy(&self.parameter, &parameter, sizeof(Parameter));

    return 0;
}

int DrvMt29Fx_Reset() {
    int result = DrvMt29Fx_WriteCommand(RESET, NULL, 0, NULL, 0);
    if (result != 0) {
        return result;
    }

    self.interface.delayMs(100);

    return 0;
}

int DrvMt29Fx_LockBlocks() {
    const uint8_t address_byte = BLOCK_LOCK;
    BlockLockRegister block_lock = {
        .tb = 1,
        .bp0 = 1,
        .bp1 = 1,
        .bp2 = 1,
        .bp3 = 1,
        .brwd = 0,
        .wp_hold_disable = 1,
        .reserved0 = 0,
    };

    int result = DrvMt29Fx_WriteCommand(SET_FEATURES, &address_byte, 1, (uint8_t*)&block_lock, 1);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_ReadCommand(GET_FEATURES, &address_byte, 1, (uint8_t*)&self.block_lock, 1);
    if (result != 0) {
        return result;
    }

    return result;
}

int DrvMt29Fx_UnlockBlocks() {
    const uint8_t address_byte = BLOCK_LOCK;
    BlockLockRegister block_lock = {
        .tb = 0,
        .bp0 = 0,
        .bp1 = 0,
        .bp2 = 0,
        .bp3 = 0,
        .brwd = 0,
        .wp_hold_disable = 1,
        .reserved0 = 0,
    };

    int result = DrvMt29Fx_WriteCommand(SET_FEATURES, &address_byte, 1, (uint8_t*)&block_lock, 1);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_ReadCommand(GET_FEATURES, &address_byte, 1, (uint8_t*)&self.block_lock, 1);
    if (result != 0) {
        return result;
    }

    return result;
}

int DrvMt29Fx_Write(const DrvMt29FxAddress* address, const uint8_t* data, uint16_t length) {
    if (!isValidAddress(address) || data == NULL || length == 0) {
        return -1;
    }

    int result = DrvMt29Fx_CheckBadBlock(address);
    if (result != 0) {
        return result;
    }

    const uint16_t size = length > self.bytes_per_page ? self.bytes_per_page : length;
    uint32_t retry_count = 0;
    const uint32_t max_retries = MAX_WRITE_RETRIES;

write_retry:
    // status.wel is automatically cleared when PROGRAM, ERASE completed
    // so, set status.wel whenever executing PROGRAM, ERASE
    result = DrvMt29Fx_EnableWrite();
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WriteToCache(address, data, size);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_TransferToMain(address);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WaitForNotInOperation();
    if (result != 0) {
        return result;
    }

    if (DrvMt29Fx_IsProgrammed() == false) {
        ++self.write_error_times;
        retry_count++;

        if (retry_count >= max_retries) {
            DrvMt29Fx_MarkBadBlock(address);
            return -3;
        }
        goto write_retry;
    }

    return DrvMt29Fx_DisableWrite();
}

int DrvMt29Fx_Read(const DrvMt29FxAddress* address, uint8_t* data, uint16_t length) {
    if (!isValidAddress(address) || data == NULL || length == 0) {
        return -1;
    }

    DrvMt29FxAddress temp_address = *address;
    temp_address.column = 0;

    const uint16_t size = length > self.bytes_per_page ? self.bytes_per_page : length;

    int result = DrvMt29Fx_TransferToCache(&temp_address);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WaitForNotInOperation();
    if (result != 0) {
        return result;
    }

    return DrvMt29Fx_ReadFromCache(&temp_address, data, size);
}

int DrvMt29Fx_Erase(const DrvMt29FxAddress* address) {
    if (!isValidAddress(address)) {
        return -1;
    }

    int result = DrvMt29Fx_CheckBadBlock(address);
    if (result != 0) {
        return result;
    }

    int retry_count = 0;
    const int max_retries = 3;

erase_retry:
    // status.wel is automatically cleared when PROGRAM, ERASE completed
    // so, set status.wel whenever executing PROGRAM, ERASE
    result = DrvMt29Fx_EnableWrite();
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_EraseBlock(address);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WaitForNotInOperation();
    if (result != 0) {
        return result;
    }

    if (DrvMt29Fx_IsErased() == false) {
        ++self.erase_error_times;
        retry_count++;

        if (retry_count >= max_retries) {
            DrvMt29Fx_MarkBadBlock(address);
            return -3;
        }
        goto erase_retry;
    }

    return DrvMt29Fx_DisableWrite();
}

int DrvMt29Fx_EraseAll(const bool is_include_bad_block) {
    int retry_count = 0;
    const int max_retries = 3;

    for (uint8_t unit = 0; unit < self.units_per_device; unit++) {
        for (uint16_t block = 0; block < self.blocks_per_unit; block++) {
            DrvMt29FxAddress address = {
                .block = block,
                .page = 0,
                .column = 0,
            };

            int result = DrvMt29Fx_CheckBadBlock(&address);
            if (!is_include_bad_block && result == -2) {
                continue;
            }

        erase_retry:
            // status.wel is automatically cleared when PROGRAM, ERASE completed
            // so, set status.wel whenever executing PROGRAM, ERASE
            result = DrvMt29Fx_EnableWrite();
            if (result != 0) {
                return result;
            }

            result = DrvMt29Fx_EraseBlock(&address);
            if (result != 0) {
                return result;
            }

            result = DrvMt29Fx_WaitForNotInOperation();
            if (result != 0) {
                return result;
            }

            if (DrvMt29Fx_IsErased() == false) {
                ++self.erase_error_times;
                retry_count++;

                if (retry_count >= max_retries) {
                    DrvMt29Fx_MarkBadBlock(&address);
                    continue;
                }
                goto erase_retry;
            }
        }
    }

    return 0;
}

int DrvMt29Fx_WritePartial(const DrvMt29FxAddress* address, const DrvMt29FxColumnPart part,
                           const uint8_t* data, uint16_t length) {
    if (!isValidAddress(address) || !isValidPart(part) || data == NULL || length == 0) {
        return -1;
    }

    DrvMt29FxAddress temp_address = {
        .block = address->block,
        .page = address->page,
        .column = part * self.bytes_per_partial_page,
    };
    const uint16_t size =
        length > self.bytes_per_partial_page ? self.bytes_per_partial_page : length;

    int result;

retry:
    // status.wel is automatically cleared when PROGRAM, ERASE completed
    // so, set status.wel whenever executing PROGRAM, ERASE
    result = DrvMt29Fx_EnableWrite();
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WriteToCache(&temp_address, data, size);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_TransferToMain(&temp_address);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WaitForNotInOperation();
    if (result != 0) {
        return result;
    }

    if (DrvMt29Fx_IsProgrammed() == false) {
        ++self.write_error_times;
        goto retry;
    }

    return DrvMt29Fx_DisableWrite();
}

int DrvMt29Fx_WriteSpare(const DrvMt29FxAddress* address, const uint8_t* data, uint16_t length) {
    if (!isValidAddress(address) || data == NULL || length == 0) {
        return -1;
    }

    int result = 0;
    int retry_count = 0;
    const int max_retries = MAX_WRITE_RETRIES;
    const uint16_t size = length > self.bytes_per_spare_page ? self.bytes_per_spare_page : length;
    const DrvMt29FxAddress spare_address = {
        .block = address->block,
        .page = address->page,
        .column = self.bytes_per_page,
    };

write_retry:
    result = DrvMt29Fx_EnableWrite();
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WriteToCache(&spare_address, data, size);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_TransferToMain(&spare_address);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WaitForNotInOperation();
    if (result != 0) {
        return result;
    }

    if (DrvMt29Fx_IsProgrammed() == false) {
        ++self.write_error_times;
        retry_count++;

        if (retry_count >= max_retries) {
            DrvMt29Fx_MarkBadBlock(address);
            return -3;
        }
        goto write_retry;
    }

    return DrvMt29Fx_DisableWrite();
}

int DrvMt29Fx_ReadSpare(const DrvMt29FxAddress* address, uint8_t* data, uint16_t length) {
    if (!isValidAddress(address) || data == NULL || length == 0) {
        return -1;
    }

    const DrvMt29FxAddress spare_address = {
        .block = address->block,
        .page = address->page,
        .column = self.bytes_per_page,
    };
    const uint16_t size = length > self.bytes_per_spare_page ? self.bytes_per_spare_page : length;

    int result = DrvMt29Fx_TransferToCache(&spare_address);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WaitForNotInOperation();
    if (result != 0) {
        return result;
    }

    return DrvMt29Fx_ReadFromCache(&spare_address, data, size);
}

int DrvMt29Fx_WriteWithSpare(const DrvMt29FxAddress* address, const uint8_t* data, uint16_t size,
                             const uint8_t* spare_meta_data_1, uint8_t spare_meta_data_1_size,
                             const uint8_t* spare_meta_data_2, uint8_t spare_meta_data_2_size) {
    if (!isValidAddress(address) || data == NULL || size != self.bytes_per_page ||
        (spare_meta_data_1 != NULL &&
         spare_meta_data_1_size != self.spare_area_user_meta_data_1_size) ||
        (spare_meta_data_2 != NULL &&
         spare_meta_data_2_size != self.spare_area_user_meta_data_2_size) ||
        (spare_meta_data_1 == NULL && spare_meta_data_2 == NULL)) {
        return -1;
    }

    int result = DrvMt29Fx_CheckBadBlock(address);
    if (result != 0) {
        return result;
    }

    int retry_count = 0;
    const int max_retries = MAX_WRITE_RETRIES;
    uint8_t buffer[self.bytes_per_page + self.bytes_per_spare_page];

    memset(buffer, NFM_GOOD_BLOCK_MARKER, sizeof(buffer));

    memcpy(buffer, data, size);

    if (spare_meta_data_1 != NULL) {
        memcpy(buffer + self.spare_area_user_meta_data_1_offset_address, spare_meta_data_1,
               spare_meta_data_1_size);
    }

    if (spare_meta_data_2 != NULL) {
        memcpy(buffer + self.spare_area_user_meta_data_2_offset_address, spare_meta_data_2,
               spare_meta_data_2_size);
    }

write_retry:
    // status.wel is automatically cleared when PROGRAM, ERASE completed
    // so, set status.wel whenever executing PROGRAM, ERASE
    result = DrvMt29Fx_EnableWrite();
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WriteToCache(address, buffer, sizeof(buffer));
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_TransferToMain(address);
    if (result != 0) {
        return result;
    }

    if (DrvMt29Fx_IsProgrammed() == false) {
        ++self.write_error_times;
        retry_count++;

        if (retry_count >= max_retries) {
            DrvMt29Fx_MarkBadBlock(address);
            return -3;
        }
        goto write_retry;
    }

    return DrvMt29Fx_DisableWrite();
}

int DrvMt29Fx_ReadWithSpare(const DrvMt29FxAddress* address, uint8_t* data, uint16_t size,
                            uint8_t* spare_meta_data_1, uint8_t spare_meta_data_1_size,
                            uint8_t* spare_meta_data_2, uint8_t spare_meta_data_2_size) {
    if (!isValidAddress(address) || data == NULL || size != self.bytes_per_page ||
        (spare_meta_data_1 != NULL &&
         spare_meta_data_1_size != self.spare_area_user_meta_data_1_size) ||
        (spare_meta_data_2 != NULL &&
         spare_meta_data_2_size != self.spare_area_user_meta_data_2_size) ||
        (spare_meta_data_1 == NULL && spare_meta_data_2 == NULL)) {
        return -1;
    }

    int result = DrvMt29Fx_CheckBadBlock(address);
    if (result != 0) {
        return result;
    }

    uint8_t buffer[self.bytes_per_page + self.bytes_per_spare_page];

    result = DrvMt29Fx_TransferToCache(address);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WaitForNotInOperation();
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_ReadFromCache(address, buffer, sizeof(buffer));
    if (result != 0) {
        return result;
    }

    memcpy(data, buffer, size);
    memcpy(spare_meta_data_1, buffer + self.spare_area_user_meta_data_1_offset_address,
           spare_meta_data_1_size);
    memcpy(spare_meta_data_2, buffer + self.spare_area_user_meta_data_2_offset_address,
           spare_meta_data_2_size);

    return result;
}

int DrvMt29Fx_WriteSpareMetas(const DrvMt29FxAddress* address, uint8_t* spare_meta_data_1,
                              uint8_t spare_meta_data_1_size, uint8_t* spare_meta_data_2,
                              uint8_t spare_meta_data_2_size) {
    if (!isValidAddress(address) ||
        (spare_meta_data_1 != NULL &&
         spare_meta_data_1_size != self.spare_area_user_meta_data_1_size) ||
        (spare_meta_data_2 != NULL &&
         spare_meta_data_2_size != self.spare_area_user_meta_data_2_size) ||
        (spare_meta_data_1 == NULL && spare_meta_data_2 == NULL)) {
        return -1;
    }

    DrvMt29FxAddress spare_address = {
        .block = address->block,
        .page = address->page,
        .column = self.bytes_per_page,
    };
    uint16_t meta_data_1_buffer_offset =
        self.spare_area_user_meta_data_1_offset_address - self.bytes_per_page;
    uint16_t meta_data_2_buffer_offset =
        self.spare_area_user_meta_data_2_offset_address - self.bytes_per_page;

    uint8_t buffer[self.bytes_per_spare_page];

    uint32_t retry_count = 0;
    const uint32_t max_retries = MAX_WRITE_RETRIES;

    memset(buffer, 0xFF, sizeof(buffer));

    if (spare_meta_data_1 != NULL) {
        memcpy(buffer + meta_data_1_buffer_offset, spare_meta_data_1, spare_meta_data_1_size);
    }

    if (spare_meta_data_2 != NULL) {
        memcpy(buffer + meta_data_2_buffer_offset, spare_meta_data_2, spare_meta_data_2_size);
    }

write_retry:
    int result = DrvMt29Fx_EnableWrite();
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WriteToCache(&spare_address, buffer, sizeof(buffer));
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_TransferToMain(&spare_address);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WaitForNotInOperation();
    if (result != 0) {
        return result;
    }

    if (DrvMt29Fx_IsProgrammed() == false) {
        ++self.write_error_times;
        retry_count++;

        if (retry_count >= max_retries) {
            DrvMt29Fx_MarkBadBlock(address);
            return -3;
        }
        goto write_retry;
    }

    return DrvMt29Fx_DisableWrite();
}

int DrvMt29Fx_ReadSpareMetas(const DrvMt29FxAddress* address, uint8_t* spare_meta_data_1,
                             uint8_t spare_meta_data_1_size, uint8_t* spare_meta_data_2,
                             uint8_t spare_meta_data_2_size) {
    if (!isValidAddress(address) ||
        (spare_meta_data_1 != NULL &&
         spare_meta_data_1_size != self.spare_area_user_meta_data_1_size) ||
        (spare_meta_data_2 != NULL &&
         spare_meta_data_2_size != self.spare_area_user_meta_data_2_size) ||
        (spare_meta_data_1 == NULL && spare_meta_data_2 == NULL)) {
        return -1;
    }

    DrvMt29FxAddress spare_address = {
        .block = address->block,
        .page = address->page,
        .column = self.bytes_per_page,
    };
    uint16_t meta_data_1_buffer_offset =
        self.spare_area_user_meta_data_1_offset_address - self.bytes_per_page;
    uint16_t meta_data_2_buffer_offset =
        self.spare_area_user_meta_data_2_offset_address - self.bytes_per_page;

    uint8_t buffer[self.bytes_per_spare_page];

    int result = DrvMt29Fx_TransferToCache(&spare_address);
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_WaitForNotInOperation();
    if (result != 0) {
        return result;
    }

    result = DrvMt29Fx_ReadFromCache(&spare_address, buffer, sizeof(buffer));
    if (result != 0) {
        return result;
    }

    memcpy(spare_meta_data_1, buffer + meta_data_1_buffer_offset,
           self.spare_area_user_meta_data_1_size);
    memcpy(spare_meta_data_2, buffer + meta_data_2_buffer_offset,
           self.spare_area_user_meta_data_2_size);

    return result;
}

int DrvMt29Fx_CheckBadBlock(const DrvMt29FxAddress* address) {
    if (!isValidAddress(address)) {
        return -1;
    }

    uint8_t marker1 = 0xFF, marker2 = 0xFF;
    DrvMt29FxAddress check_address = *address;

    check_address.page = 0;
    check_address.column = self.bytes_per_page;

    int result = DrvMt29Fx_ReadSpare(&check_address, &marker1, 1);
    if (result != 0) {
        return -1;
    }

    if (self.pages_per_block > 1) {
        check_address.page = 1;

        result = DrvMt29Fx_ReadSpare(&check_address, &marker2, 1);
        if (result != 0) {
            return -1;
        }
    }

    return (marker1 != 0xFF || marker2 != 0xFF) ? -2 : 0;
}

int DrvMt29Fx_MarkBadBlock(const DrvMt29FxAddress* address) {
    if (!isValidAddress(address)) {
        return -1;
    }

    const uint8_t marker = NFM_BAD_BLOCK_MARKER;
    DrvMt29FxAddress spare_address = *address;

    for (uint16_t page = 0; page < 2 && page < self.pages_per_block; page++) {
        spare_address.page = page;
        spare_address.column = self.bytes_per_page;

        int result = DrvMt29Fx_EnableWrite();
        if (result != 0) {
            return result;
        }

        result = DrvMt29Fx_WriteToCache(&spare_address, &marker, 1);
        if (result != 0) {
            return result;
        }

        result = DrvMt29Fx_TransferToMain(&spare_address);
        if (result != 0) {
            return result;
        }

        result = DrvMt29Fx_WaitForNotInOperation();
        if (result != 0) {
            return result;
        }

        result = DrvMt29Fx_DisableWrite();
        if (result != 0) {
            return result;
        }
    }

    return 0;
}

int DrvMt29Fx_ScanBadBlocks(uint16_t* bad_blocks, uint16_t* count, uint16_t max_count) {
    if (bad_blocks == NULL || count == NULL) {
        return -1;
    }

    *count = 0;

    for (uint8_t unit = 0; unit < self.units_per_device; unit++) {
        for (uint16_t block = 0; block < self.blocks_per_unit; block++) {
            DrvMt29FxAddress address = {
                .block = block,
                .page = 0,
                .column = 0,
            };

            int result = DrvMt29Fx_CheckBadBlock(&address);
            if (result == -2) {
                if (*count < max_count) {
                    bad_blocks[*count] = (unit * self.blocks_per_unit) + block;
                    (*count)++;
                }
            } else if (result == -1) {
                return -1;
            }
        }
    }

    return 0;
}

int DrvMt29Fx_RegisterInterface(DrvMt29FxInterface* itf) {
    if (itf == NULL || itf->write == NULL || itf->read == NULL || itf->delayMs == NULL ||
        itf->setCsPin == NULL || itf->setWpPin == NULL || itf->setHoldPin == NULL) {
        return -1;
    }

    DrvMt29Fx_SetInterface(itf);

    return 0;
}

uint16_t DrvMt29Fx_GetBytesPerPage() { return self.bytes_per_page; }

uint16_t DrvMt29Fx_GetBytesPerSpareArea() { return self.bytes_per_spare_page; }

uint16_t DrvMt29Fx_GetBytesPerPartialPage() { return self.bytes_per_partial_page; }

uint16_t DrvMt29Fx_GetPagesPerBlock() { return self.pages_per_block; }

uint16_t DrvMt29Fx_GetBlocksPerUnit() { return self.blocks_per_unit; }

uint16_t DrvMt29Fx_GetUnitsPerDevice() { return self.units_per_device; }

uint16_t DrvMt29Fx_GetPlanesPerDevice() { return self.planes_per_device; }

uint32_t DrvMt29Fx_GetBlockSize() { return self.bytes_per_page * self.pages_per_block; }

uint32_t DrvMt29Fx_GetPageSize() { return self.bytes_per_page; }

uint32_t DrvMt29Fx_GetUnitSize() {
    return self.bytes_per_page * self.pages_per_block * self.blocks_per_unit;
}

uint8_t DrvMt29Fx_GetSpareAreaUserMetaData1Size() { return self.spare_area_user_meta_data_1_size; }

uint8_t DrvMt29Fx_GetSpareAreaUserMetaData2Size() { return self.spare_area_user_meta_data_2_size; }
