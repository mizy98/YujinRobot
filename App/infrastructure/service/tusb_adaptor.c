#include "tusb_adaptor.h"

// CDC end points : notification(IN), data OUT, data IN
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

#define EPNUM_CDC_NOTIF (0x81)  // EP1 IN  (notification)
#define EPNUM_CDC_OUT (0x02)    // EP2 OUT (data)
#define EPNUM_CDC_IN (0x82)     // EP2 IN  (data)

static uint16_t _desc_str_buffer[32] = {0};

static char _device_serial_number[25] = {0};

// USB Device configuration descriptor
uint8_t const desc_configuration[] = {
    // Config descriptor
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // CDC descriptor (IAD + Communication + Data interface)
    TUD_CDC_DESCRIPTOR(0, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};

// USB Device descriptor
tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,  // USB 2.0

    // CDC: class는 IAD를 위해 0xEF
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0xCafe,  // 개발용 VID
    .idProduct = 0x4001,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01,
};

char const* string_desc_arr[] = {
    (const char[]){0x09, 0x04},  // [0] Language: English (0x0409)
    APP_VENDOR_NAME,             // [1] Manufacturer
    APP_PROJECT_NAME,            // [2] Product
    _device_serial_number,       // [3] Serial Number
    APP_PROJECT_NAME " CDC",     // [4] CDC Interface
};

void tusb_generate_serial_number(void) {
    if (_device_serial_number[0] != 0) return;

    uint32_t uid[3];
    uid[0] = *(volatile uint32_t*)(UID_BASE);
    uid[1] = *(volatile uint32_t*)(UID_BASE + 4);
    uid[2] = *(volatile uint32_t*)(UID_BASE + 8);

    // 96비트 전체를 24자 16진수로 변환
    snprintf(_device_serial_number, sizeof(_device_serial_number), "%08lX%08lX%08lX", uid[0],
             uid[1], uid[2]);
}
void TusbAdaptor_Initialize(void) { tusb_generate_serial_number(); }

// USB Device callback: descriptor configuration callback
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

// USB Device callback: descriptor device callback
uint8_t const* tud_descriptor_device_cb(void) { return (uint8_t const*)&desc_device; }

// USB Device callback: descriptor string callback
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str_buffer[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) return NULL;
        const char* str = string_desc_arr[index];
        chr_count = (uint8_t)strlen(str);
        if (chr_count > 31) chr_count = 31;
        for (uint8_t i = 0; i < chr_count; i++) _desc_str_buffer[1 + i] = str[i];
    }

    _desc_str_buffer[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return _desc_str_buffer;
}

// USB Device CDC callback: when the host changes line coding (baud rate, etc.)
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding) {
    (void)itf;
    // 필요 시 UART 보레이트 등 재설정
    // p_line_coding->bit_rate, stop_bits, parity, data_bits
}
