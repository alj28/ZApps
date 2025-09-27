
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/util.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usb_hid.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_inputs, LOG_LEVEL_INF);


static void input_cb(struct input_event *evt, void *user_data)
{
	ARG_UNUSED(user_data);

    LOG_INF("input_cb: evt->code: %d, evt->value: %d", evt->code, evt->value);

}

/*  This callback function will handle all system inputs.
 */
INPUT_CALLBACK_DEFINE(NULL, input_cb, NULL);