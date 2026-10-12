#ifndef COMM_HGP_CAN_H
#define COMM_HGP_CAN_H

#include "comm_hgp_define.h"

int CommHgpCan_Initialize(const CommHgpConfig* config, const CommHgpCanInterface* interface,
                          const CommHgpCanCallback* callback);
int CommHgpCan_Deinitialize(void);
int CommHgpCan_Parse(uint32_t id, uint8_t* data, uint16_t length);
void CommHgpCan_ResetParseState(void);
int CommHgpCan_SendData(CommHgpData* data);

#endif
