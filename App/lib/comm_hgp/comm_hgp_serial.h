#ifndef COMM_HGP_SERIAL_H
#define COMM_HGP_SERIAL_H

#include "comm_hgp_define.h"

int CommHgpSerial_Initialize(const CommHgpConfig* config, const CommHgpSerialInterface* interface,
                             const CommHgpSerialCallback* callback);
int CommHgpSerial_Deinitialize(void);
int CommHgpSerial_Parse(uint8_t* data, uint16_t length);
void CommHgpSerial_ResetParseState(void);

#endif