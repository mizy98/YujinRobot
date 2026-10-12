#include "comm_hgp_can.h"

#define self (comm_hgp_can_private)

#define ORDER_COMMAND_STREAM (0xAA)

#define MODE_AUTO_START (0xE0)
#define MODE_AUTO_STOP (0x60)

static struct {
    uint8_t device_address;
    uint32_t device_can_id;

    bool is_data_streaming;

    CommHgpCanCallback callback;
    CommHgpCanInterface interface;
} comm_hgp_can_private;

int CommHgpCan_Initialize(const CommHgpConfig* config, const CommHgpCanInterface* interface,
                          const CommHgpCanCallback* callback) {
    if (config == NULL || interface == NULL || interface->write == NULL || callback == NULL ||
        callback->onDataRequested == NULL || callback->onDataStreamStartRequested == NULL ||
        callback->onDataStreamStopRequested == NULL) {
        return COMM_HGP_FAIL;
    }

    self.callback = *callback;
    self.interface = *interface;
    self.device_address = config->device_address;
    self.device_can_id = config->device_address | HGP_CAN_BASE_ID;

    return COMM_HGP_OK;
}

int CommHgpCan_Deinitialize(void) { return COMM_HGP_OK; }

int CommHgpCan_Parse(uint32_t id, uint8_t* data, uint16_t length) {
    if (id != self.device_can_id || length > 8 || length < 1) {
        return COMM_HGP_FAIL;
    }

    uint8_t rx_order = data[0];

    if (rx_order == self.device_address) {
        self.callback.onDataRequested();

    } else if (rx_order == ORDER_COMMAND_STREAM) {
        uint8_t mode = data[1];

        if ((mode & MODE_AUTO_START) == MODE_AUTO_START) {
            self.is_data_streaming = true;

            self.callback.onDataStreamStartRequested();

        } else if ((mode & MODE_AUTO_STOP) == MODE_AUTO_STOP) {
            self.is_data_streaming = false;
            self.callback.onDataStreamStopRequested();
        }
    }

    return COMM_HGP_OK;
}

void CommHgpCan_ResetParseState(void) {}

int CommHgpCan_SendData(CommHgpData* data) {
    if (data == NULL) {
        return COMM_HGP_FAIL;
    }

    uint8_t buffer[3][8] = {0};

    buffer[0][0] = self.device_address;
    buffer[0][1] = 1;  // index;
    buffer[0][2] = data->pack_voltage & 0xFF;
    buffer[0][3] = data->pack_voltage >> 8;
    buffer[0][4] = data->pack_current & 0xFF;
    buffer[0][5] = data->pack_current >> 8;
    buffer[0][6] = data->state.value & 0xFF;
    buffer[0][7] = data->state.value >> 8;

    buffer[1][0] = self.device_address;
    buffer[1][1] = 2;  // index;
    buffer[1][2] = data->charge_remaining_time & 0xFF;
    buffer[1][3] = data->charge_remaining_time >> 8;
    buffer[1][4] = data->discharge_remaining_time & 0xFF;
    buffer[1][5] = data->discharge_remaining_time >> 8;
    buffer[1][6] = data->soc;
    buffer[1][7] = data->soh;

    buffer[2][0] = self.device_address;
    buffer[2][1] = 3;  // index;
    buffer[2][2] = data->remaining_capacity & 0xFF;
    buffer[2][3] = data->remaining_capacity >> 8;
    buffer[2][4] = data->remaining_energy & 0xFF;
    buffer[2][5] = data->remaining_energy >> 8;
    buffer[2][6] = data->temperature & 0xFF;
    buffer[2][7] = data->temperature >> 8;

    int result = 0;
    for (uint8_t i = 0; i < 3; i++) {
        if (self.interface.write(self.device_can_id, 0, 0, buffer[i], 8) != 0) {
            result = COMM_HGP_FAIL;
            continue;
        }
    }

    if (result != 0) {
        return COMM_HGP_FAIL;
    }

    return COMM_HGP_OK;
}