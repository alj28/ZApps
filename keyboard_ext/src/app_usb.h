#ifndef _APP_USB_H_
#define _APP_USB_H_

#include <zephyr/kernel.h>

int app_usb_report_key_press(uint32_t code, bool is_pressed, k_timeout_t timeout);

#endif