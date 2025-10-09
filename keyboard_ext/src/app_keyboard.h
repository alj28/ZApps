#ifndef _APP_KEYBOARD_H_
#define _APP_KEYBOARD_H_

#include <zephyr/kernel.h>

#include "config.h"

int app_keyboard_report_key_press(uint32_t code, bool is_pressed, k_timeout_t timeout);
int app_keyboard_get_pressed(uint32_t codes[CONFIG_APP_MAX_KEYS_REPORTED], size_t* len, k_timeout_t timeout);

#endif