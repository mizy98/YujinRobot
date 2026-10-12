#ifndef BMS_PROTECTION_STATE_H
#define BMS_PROTECTION_STATE_H

#include <stdint.h>

#pragma pack(push, 1)
typedef union {
    struct {
        uint32_t ovp : 1;
        uint32_t uvp : 1;
        uint32_t cocp : 1;
        uint32_t docp : 1;
        uint32_t rocp : 1;
        uint32_t otp : 1;
        uint32_t cotp : 1;
        uint32_t dotp : 1;
        uint32_t rotp : 1;
        uint32_t utp : 1;
        uint32_t cutp : 1;
        uint32_t dutp : 1;
        uint32_t rutp : 1;
        uint32_t odvp : 1;
        uint32_t codvp : 1;
        uint32_t dodvp : 1;
        uint32_t rodvp : 1;
        uint32_t odtp : 1;
        uint32_t opp : 1;
        uint32_t upp : 1;
        uint32_t : 12;
    };
    uint32_t value;
} BmsProtectionState;
#pragma pack(pop)

#endif