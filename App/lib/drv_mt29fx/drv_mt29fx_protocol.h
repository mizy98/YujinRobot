#ifndef DRV_MT29FX_PROTOCOL_H
#define DRV_MT29FX_PROTOCOL_H

#include "drv_mt29fx_define.h"

// MT29FX Commands
#define RESET 0xFF
#define GET_FEATURES 0x0F
#define SET_FEATURES 0x1F
#define READ_ID 0x9F
#define PAGE_READ 0x13
#define READ_PAGE_CACHE_RANDOM 0x30
#define READ_PAGE_CACHE_LAST 0x3F
#define READ_FROM_CACHE_X1 0x03
#define READ_FROM_CACHE_X2 0x3B
#define READ_FROM_CACHE_X4 0x6B
#define READ_FROM_CACHE_DUAL_IO 0xBB
#define READ_FROM_CACHE_QUAD_IO 0xEB
#define WRITE_ENABLE 0x06
#define WRITE_DISABLE 0x04
#define BLOCK_ERASE 0xD8
#define PROGRAM_EXECUTE 0x10
#define PROGRAM_LOAD_X1 0x02
#define PROGRAM_LOAD_X4 0x32
#define PROGRAM_LOAD_RANDOM_DATA_X1 0x84
#define PROGRAM_LOAD_RANDOM_DATA_X4 0x34
#define PROGRAM_PERMANENT_BLOCK_LOCK_PROTECTION 0x2C

#define COLUMN_ADDRESS_PLANE_MASK (12u)
#define ROW_ADDRESS_BLOCK_MASK (6u)

#define NFM_BAD_BLOCK_MARKER (0x00)
#define NFM_GOOD_BLOCK_MARKER (0xFF)

#define MAX_WRITE_RETRIES (3u)
#define TIMEOUT_TRY_COUNT (5000u)

typedef enum {
    BLOCK_LOCK = 0xA0,
    CONFIGURATION = 0xB0,
    STATUS = 0xC0,
    DIE_SELECT = 0xD0
} FeatureAddressByte;

typedef enum {
    MODEL_INVALID,
    MODEL_MT29F1G01,
    MODEL_MT29F2G01,
    MODEL_MT29F4G01,
    MODEL_MT29F8G01
} Model;

#pragma pack(push, 1)
typedef struct {
    uint8_t parameter_signature[4];
    uint16_t revision_number;
    uint16_t feature_support;
    uint16_t optional_commands_support;
    uint8_t reserved1[22];
    uint8_t device_manufacturer[12];
    uint8_t device_model[20];
    uint8_t manufacturer_id;
    uint16_t date_code;
    uint8_t reserved2[13];
    uint32_t num_data_bytes_per_page;
    uint16_t num_spare_bytes_per_page;
    uint32_t num_data_bytes_per_partial_page;
    uint16_t num_spare_bytes_per_partial_page;
    uint32_t num_pages_per_block;
    uint32_t num_blocks_per_unit;
    uint8_t num_logical_units;
    uint8_t num_address_cycles;
    uint8_t num_bits_per_cell;
    uint16_t bad_blocks_max_per_unit;
    uint16_t block_endurance;
    uint8_t guaranteed_valid_blocks;
    uint16_t block_endurance_valid_blocks;
    uint8_t num_programs_per_page;
    uint8_t partial_programming_attributes;
    uint8_t num_ecc_bits;
    uint8_t num_interleaved_address_bits;
    uint8_t interleaved_operation_attributes;
    uint8_t reserved3[13];
    uint8_t io_pin_capacitance;
    uint16_t timing_mode_support;
    uint16_t program_cache_timing;
    uint16_t t_prog_max_page_program_time;
    uint16_t t_ers_max_block_erase_time;
    uint16_t t_r_max_page_read_time;
    uint16_t t_ccs_min;
    uint8_t reserved4[23];
    uint16_t vendor_specific_revision_number;
    uint8_t vendor_specific[14];
    uint8_t reserved5[68];
    uint8_t ecc_max_correct_ability;
    uint8_t die_select_feature;
    uint8_t reserved6[4];
    uint16_t integrity_crc;
} Parameter;  // ONFI (Open NAND Flash Interface)

typedef struct {
    union {
        struct {
            uint8_t reserved0 : 1;
            uint8_t wp_hold_disable : 1;
            uint8_t tb : 1;
            uint8_t bp0 : 1;
            uint8_t bp1 : 1;
            uint8_t bp2 : 1;
            uint8_t bp3 : 1;
            uint8_t brwd : 1;
        };
        uint8_t value;
    };
} BlockLockRegister;

typedef struct {
    union {
        struct {
            uint8_t reserved0 : 1;
            uint8_t cfg0 : 1;
            uint8_t reserved1 : 1;
            uint8_t reserved2 : 1;
            uint8_t ecc_en : 1;
            uint8_t lot_en : 1;
            uint8_t cfg1 : 1;
            uint8_t cfg2 : 1;
        };
        uint8_t value;
    };

} ConfigurationRegister;

typedef struct {
    union {
        struct {
            uint8_t oip : 1;
            uint8_t wel : 1;
            uint8_t e_fail : 1;
            uint8_t p_fail : 1;
            uint8_t eccs0 : 1;
            uint8_t eccs1 : 1;
            uint8_t eccs2 : 1;
            uint8_t crbsy : 1;
        };
        uint8_t value;
    };
} StatusRegister;

typedef struct {
    union {
        struct {
            uint8_t reserved1 : 6;
            uint8_t ds0 : 1;
            uint8_t reserved0 : 1;
        };
        uint8_t value;
    };
} DieSelectRegister;
#pragma pack(pop)

void DrvMt29Fx_SetInterface(DrvMt29FxInterface* interface);
void DrvMt29Fx_RegisterPlaneInfo(uint8_t max_count);
void DrvMt29Fx_SetWriteProtection(uint8_t state);

int DrvMt29Fx_WriteCommand(const uint8_t command, const uint8_t* address_bytes,
                           const uint8_t address_byte_length, const uint8_t* data,
                           const uint16_t data_length);

int DrvMt29Fx_ReadCommand(const uint8_t command, const uint8_t* address_bytes,
                          const uint8_t address_byte_length, uint8_t* data,
                          const uint16_t data_length);

int DrvMt29Fx_WaitForNotInOperation();
bool DrvMt29Fx_IsProgrammed();
bool DrvMt29Fx_IsErased();

int DrvMt29Fx_ReadStatus(StatusRegister* status);
int DrvMt29Fx_ReadBlockLockStatus(BlockLockRegister* block_lock);
int DrvMt29Fx_ReadConfiguration(ConfigurationRegister* configuration);
int DrvMt29Fx_ReadDieSelect(DieSelectRegister* die_select);
int DrvMt29Fx_ReadDeviceId(uint8_t* manufacturer_id, uint8_t* device_id);
int DrvMt29Fx_ReadParameter(Parameter* parameter, const ConfigurationRegister* configuration);

int DrvMt29Fx_WriteConfiguration(const ConfigurationRegister* configuration);

int DrvMt29Fx_EnableWrite();
int DrvMt29Fx_DisableWrite();

int DrvMt29Fx_TransferToCache(const DrvMt29FxAddress* address);
int DrvMt29Fx_TransferToMain(const DrvMt29FxAddress* address);

int DrvMt29Fx_WriteToCache(const DrvMt29FxAddress* address, const uint8_t* data,
                           const uint16_t length);
int DrvMt29Fx_ReadFromCache(const DrvMt29FxAddress* address, uint8_t* data, const uint16_t length);
int DrvMt29Fx_EraseBlock(const DrvMt29FxAddress* address);

uint32_t DrvMt29Fx_ToRowAddress(const DrvMt29FxAddress* address);
uint32_t DrvMt29Fx_ToMultiPlaneColumnAddress(const DrvMt29FxAddress* address);

#endif  // DRV_MT29FX_PROTOCOL_H
