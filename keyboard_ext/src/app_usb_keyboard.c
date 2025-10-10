
#include <string.h>
#include <assert.h>
#include "app_usb_keyboard.h"
#include "app_keyboard.h"

int app_usb_keyboard_generate_packet(struct usb_hid_packet* packet)
{
    assert(packet);

    uint32_t key_codes[CONFIG_APP_MAX_KEYS_REPORTED];
    size_t n_key_codes = 0;

    int rv = 0;

    do {
        rv = app_keyboard_get_pressed(key_codes, &n_key_codes, K_MSEC(0));
        if (0 != rv) {
            break;
        }

        memset(packet, 0x00, sizeof(struct usb_hid_packet));

        for (size_t i = 0; ((i < CONFIG_APP_MAX_KEYS_REPORTED) && (i < n_key_codes)); i++) {
            packet->keycodes[i] = (uint8_t)(key_codes[i] & 0x000000FF);
        }
    } while(false);

    return rv;
}

