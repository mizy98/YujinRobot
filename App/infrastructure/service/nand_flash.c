#include "nand_flash.h"

#if defined(USE_HAL_DRIVER)

#include "../../lib/drv_mt29fx/drv_mt29fx.h"

#define self (nand_flash_private)

#define SPI_FAIL (-1)
#define SPI_TIMEOUT (-2)

typedef enum {
    SPI_EVENT_TX_COMPLETE = 0x01,
    SPI_EVENT_RX_COMPLETE = 0x02,
    SPI_EVENT_ERROR = 0x04,
} SpiEvent;

static struct {
    AppErrorInfo error_info;
    bool is_initialized;

    osEventFlagsId_t spi_event;
} nand_flash_private;

static void _onTxComplete(SPI_HandleTypeDef* hspi) {
    osEventFlagsSet(self.spi_event, SPI_EVENT_TX_COMPLETE);
}

static void _onRxComplete(SPI_HandleTypeDef* hspi) {
    osEventFlagsSet(self.spi_event, SPI_EVENT_RX_COMPLETE);
}

static void _onError(SPI_HandleTypeDef* hspi) { osEventFlagsSet(self.spi_event, SPI_EVENT_ERROR); }

static int _write(uint8_t* data, uint16_t length) {
    if (HAL_SPI_Transmit_DMA(SPI_NAND_FLASH, data, length) != HAL_OK) {
        return SPI_FAIL;
    }

    uint32_t flag = SPI_EVENT_TX_COMPLETE | SPI_EVENT_ERROR;
    flag = osEventFlagsWait(self.spi_event, flag, osFlagsWaitAny, 100);
    if (flag & osFlagsError) {
        return flag == osFlagsErrorTimeout ? SPI_TIMEOUT : SPI_FAIL;
    } else if (flag & SPI_EVENT_ERROR) {
        return SPI_FAIL;
    }

    return 0;
}

static int _read(uint8_t* data, uint16_t length) {
    if (HAL_SPI_Receive_DMA(SPI_NAND_FLASH, data, length) != HAL_OK) {
        return SPI_FAIL;
    }

    uint32_t flag = SPI_EVENT_RX_COMPLETE | SPI_EVENT_ERROR;
    flag = osEventFlagsWait(self.spi_event, flag, osFlagsWaitAny, 100);
    if (flag & osFlagsError) {
        return flag == osFlagsErrorTimeout ? SPI_TIMEOUT : SPI_FAIL;
    } else if (flag & SPI_EVENT_ERROR) {
        return SPI_FAIL;
    }

    return 0;
}

static void _setCsPin(uint8_t level) {
    HAL_GPIO_WritePin(SPI2_NAND_CS_GPIO_Port, SPI2_NAND_CS_Pin,
                      level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void _setWpPin(uint8_t level) {
    HAL_GPIO_WritePin(OUT_NAND_WP_GPIO_Port, OUT_NAND_WP_Pin,
                      level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void _setHoldPin(uint8_t level) {
    HAL_GPIO_WritePin(OUT_NAND_HOLD_GPIO_Port, OUT_NAND_HOLD_Pin,
                      level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

AppResult NandFlash_Initialize(void) {
    if (HAL_SPI_GetState(SPI_NAND_FLASH) == HAL_SPI_STATE_RESET) {
        if (HAL_SPI_Init(SPI_NAND_FLASH) != HAL_OK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_NAND_FLASH_HAL_INIT);
            return APP_RESULT_FAIL;
        }
    }

    (void)HAL_SPI_RegisterCallback(SPI_NAND_FLASH, HAL_SPI_TX_COMPLETE_CB_ID, _onTxComplete);
    (void)HAL_SPI_RegisterCallback(SPI_NAND_FLASH, HAL_SPI_RX_COMPLETE_CB_ID, _onRxComplete);
    (void)HAL_SPI_RegisterCallback(SPI_NAND_FLASH, HAL_SPI_ERROR_CB_ID, _onError);

    if (self.spi_event == NULL) {
        osEventFlagsId_t event = osEventFlagsNew(NULL);
        if (event == NULL) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_NAND_FLASH_SPI_EVENT_INIT);
            return APP_RESULT_FAIL;
        }
        self.spi_event = event;
    }

    if (DrvMt29Fx_Initialize(&(DrvMt29FxInterface){
            .write = _write,
            .read = _read,
            .delayMs = (void (*)(uint32_t))osDelay,
            .setCsPin = _setCsPin,
            .setWpPin = _setWpPin,
            .setHoldPin = _setHoldPin,
        }) != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_NAND_FLASH_DRV_INIT);
        return APP_RESULT_FAIL;
    }

    if (DrvMt29Fx_UnlockBlocks() != 0) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_NAND_FLASH_UNLOCK);
        return APP_RESULT_FAIL;
    }

    self.is_initialized = true;
    return APP_RESULT_OK;
}

AppResult NandFlash_Deinitialize(void) {
    if (HAL_SPI_DeInit(SPI_NAND_FLASH) != HAL_OK) {
        APP_ERROR_SET_INFO(self.error_info, APP_ERROR_NAND_FLASH_HAL_DEINIT);
        return APP_RESULT_FAIL;
    }

    if (self.spi_event != NULL) {
        if (osEventFlagsDelete(self.spi_event) != osOK) {
            APP_ERROR_SET_INFO(self.error_info, APP_ERROR_NAND_FLASH_SPI_EVENT_DEINIT);
            return APP_RESULT_FAIL;
        }
        self.spi_event = NULL;
    }

    self.is_initialized = false;
    return APP_RESULT_OK;
}

void NandFlash_GetLastErrorInfo(AppErrorInfo* info) {
    if (info != NULL) {
        *info = self.error_info;
    }
}

bool NandFlash_IsInitialized(void) { return self.is_initialized; }

void NandFlash_GetInfo(NandFlashInfo* info) {
    if (info != NULL) {
        info->block_count = DrvMt29Fx_GetBlocksPerUnit() * DrvMt29Fx_GetUnitsPerDevice();
        info->page_count_per_block = DrvMt29Fx_GetPagesPerBlock();
        info->page_size = DrvMt29Fx_GetPageSize();
    }
}

AppResult NandFlash_Write(const NandFlashAddress* address, const uint8_t* data, uint16_t size) {
    DrvMt29FxAddress addr = {
        .block = address->block,
        .page = address->page,
        .column = 0,
    };

    if (DrvMt29Fx_Write(&addr, data, size) != 0) {
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult NandFlash_Read(const NandFlashAddress* address, uint8_t* data, uint16_t size) {
    DrvMt29FxAddress addr = {
        .block = address->block,
        .page = address->page,
        .column = 0,
    };

    if (DrvMt29Fx_Read(&addr, data, size) != 0) {
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult NandFlash_Erase(const NandFlashAddress* address) {
    DrvMt29FxAddress addr = {
        .block = address->block,
        .page = 0,
        .column = 0,
    };

    if (DrvMt29Fx_Erase(&addr) != 0) {
        return APP_RESULT_FAIL;
    }

    return APP_RESULT_OK;
}

AppResult NandFlash_CheckBadBlock(uint16_t block, bool* is_bad) {
    if (is_bad == NULL) {
        return APP_RESULT_INVALID_ARG;
    }

    int result = DrvMt29Fx_CheckBadBlock(&(DrvMt29FxAddress){
        .block = block,
        .page = 0,
        .column = 0,
    });

    if (result == -2) {
        *is_bad = true;
        return APP_RESULT_OK;
    } else if (result == -1) {
        return APP_RESULT_FAIL;
    }

    *is_bad = false;
    return APP_RESULT_OK;
}

#endif