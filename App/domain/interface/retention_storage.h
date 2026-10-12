#ifndef RETENTION_STORAGE_H
#define RETENTION_STORAGE_H

#include "../model/app_model.h"

/**
 * @details
 * Assuming a storage with large write and erase endurance. (such as FRAM or MRAM)
 * This interface provides key-value storage functionality.
 */

typedef struct {
    uint8_t key_count;  // Maximum key value + 1
    uint8_t data_size;  // Size of data for each key
} RetentionStorageInfo;

AppResult RetentionStorage_Initialize();
AppResult RetentionStorage_Deinitialize();
void RetentionStorage_GetLastErrorInfo(AppErrorInfo* info);
bool RetentionStorage_IsFormatOccurred(void);
AppResult RetentionStorage_GetInfo(RetentionStorageInfo* info);
AppResult RetentionStorage_Save(uint8_t key, uint8_t* data, uint8_t size);
AppResult RetentionStorage_Load(uint8_t key, uint8_t* data, uint8_t size);
AppResult RetentionStorage_Erase(uint8_t key);

#endif