#ifndef _APP_USB_H_
#define _APP_USB_H_

#include <zephyr/kernel.h>

void app_usb_report_key_press(void);
int app_usb_print_descriptor(void);

int app_usb_play_pause(void);
int app_usb_volume_up(void);
int app_usb_volume_down(void);
int app_usb_mute(void);

#endif