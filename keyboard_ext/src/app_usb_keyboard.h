#ifndef _APP_USB_KEYBOARD_H_
#define _APP_USB_KEYBOARD_H_

#include <stdint.h>
#include "config.h"

struct usb_hid_packet {
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycodes[CONFIG_APP_MAX_KEYS_REPORTED];
} __packed;

#define APP_USB_KEYBOARD_PACKET_SIZE        sizeof(struct usb_hid_packet)

int app_usb_keyboard_generate_packet(struct usb_hid_packet* packet);

#endif 