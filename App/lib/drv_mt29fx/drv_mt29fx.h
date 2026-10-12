#ifndef DRV_MT29FX_H
#define DRV_MT29FX_H

#include "drv_mt29fx_define.h"

/*
 * @retval
 *  0: Success
 * -1: Failure (Invalid parameters or initialization error)
 * -2: Bad block detected
 * -3: Program failed, marked as bad block
 */

int DrvMt29Fx_Initialize(DrvMt29FxInterface* itf);
int DrvMt29Fx_Reset();
int DrvMt29Fx_LockBlocks();
int DrvMt29Fx_UnlockBlocks();

int DrvMt29Fx_Write(const DrvMt29FxAddress* address, const uint8_t* data, uint16_t length);
int DrvMt29Fx_Read(const DrvMt29FxAddress* address, uint8_t* data, uint16_t length);
int DrvMt29Fx_Erase(const DrvMt29FxAddress* address);
int DrvMt29Fx_EraseAll(const bool is_inlcude_bad_block);
int DrvMt29Fx_WritePartial(const DrvMt29FxAddress* address, const DrvMt29FxColumnPart part,
                           const uint8_t* data, uint16_t length);

// Spare meta data area 1: ECC protected
// Spare meta data area 2: No ECC protection
int DrvMt29Fx_WriteSpare(const DrvMt29FxAddress* address, const uint8_t* data, uint16_t length);
int DrvMt29Fx_ReadSpare(const DrvMt29FxAddress* address, uint8_t* data, uint16_t length);
int DrvMt29Fx_WriteWithSpare(const DrvMt29FxAddress* address, const uint8_t* data, uint16_t size,
                             const uint8_t* spare_meta_data_1, uint8_t spare_meta_data_1_size,
                             const uint8_t* spare_meta_data_2, uint8_t spare_meta_data_2_size);
int DrvMt29Fx_ReadWithSpare(const DrvMt29FxAddress* address, uint8_t* data, uint16_t size,
                            uint8_t* spare_meta_data_1, uint8_t spare_meta_data_1_size,
                            uint8_t* spare_meta_data_2, uint8_t spare_meta_data_2_size);

int DrvMt29Fx_CheckBadBlock(const DrvMt29FxAddress* address);
int DrvMt29Fx_MarkBadBlock(const DrvMt29FxAddress* address);
int DrvMt29Fx_ScanBadBlocks(uint16_t* bad_blocks, uint16_t* count, uint16_t max_count);

int DrvMt29Fx_RegisterInterface(DrvMt29FxInterface* interface);

uint16_t DrvMt29Fx_GetBytesPerPage();
uint16_t DrvMt29Fx_GetBytesPerSpareArea();
uint16_t DrvMt29Fx_GetBytesPerPartialPage();
uint16_t DrvMt29Fx_GetPagesPerBlock();
uint16_t DrvMt29Fx_GetBlocksPerUnit();
uint16_t DrvMt29Fx_GetUnitsPerDevice();
uint16_t DrvMt29Fx_GetPlanesPerDevice();

uint32_t DrvMt29Fx_GetBlockSize();
uint32_t DrvMt29Fx_GetPageSize();
uint32_t DrvMt29Fx_GetUnitSize();
uint8_t DrvMt29Fx_GetSpareAreaUserMetaData1Size();
uint8_t DrvMt29Fx_GetSpareAreaUserMetaData2Size();

#endif  // DRV_MT29FX_H