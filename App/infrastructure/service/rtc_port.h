#ifndef RTC_PORT_H
#define RTC_PORT_H

#include "../../domain/model/app_model.h"
#include "platform.h"

AppResult RtcPort_Initialize(void);
AppResult RtcPort_Deinitialize(void);
bool RtcPort_IsInitialized(void);
void RtcPort_GetLastErrorInfo(AppErrorInfo* info);
AppResult RtcPort_SetTimestamp(uint64_t timestamp);
AppResult RtcPort_GetTimestamp(uint64_t* timestamp);
AppResult RtcPort_ReadBackupRegister(uint8_t index, uint32_t* data);
AppResult RtcPort_WriteBackupRegister(uint8_t index, uint32_t data);
void RtcPort_GetBackupRegisterInfo(uint8_t* register_count, uint8_t* register_size);
bool RtcPort_IsBackupFormatOccurred(void);

#endif