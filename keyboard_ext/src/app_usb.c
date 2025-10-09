
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/util.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usb_hid.h>

#include "config.h"
#include "app_keyboard.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_usb, LOG_LEVEL_INF);

static const uint8_t hid_report_desc[] = HID_KEYBOARD_REPORT_DESC();
static enum usb_dc_status_code usb_status;

static K_SEM_DEFINE(ep_write_sem, 0, 1);
static K_SEM_DEFINE(new_keyboard_state, 1, 1);

static inline void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
	usb_status = status;
}

static void int_in_ready_cb(const struct device *dev)
{
	ARG_UNUSED(dev);
	k_sem_give(&ep_write_sem);
}

static const struct hid_ops ops = {
	.int_in_ready = int_in_ready_cb,
};


void app_usb_report_key_press(void)
{
    k_sem_give(&new_keyboard_state);
}

static int app_usb_get_key_press_evt(k_timeout_t timeout)
{
    return k_sem_take(&new_keyboard_state, timeout);
}


K_THREAD_STACK_DEFINE(app_usb_thread_stack, CONFIG_APP_USB_THREAD_STACK_SIZE);
struct k_thread app_usb_thread_data;

static void app_usb_thread(void*, void*, void*)
{

    while (true) {
        int rv = app_usb_get_key_press_evt(K_FOREVER);
        if (0 != rv) {
            continue;
        }

        uint32_t keys_pressed[CONFIG_APP_MAX_KEYS_REPORTED] = {0};
        size_t n_keys_pressed = 0;
        rv = app_keyboard_get_pressed(keys_pressed, &n_keys_pressed, K_MSEC(0));

        LOG_INF("Pressed keys:");
        for (size_t i = 0; i < n_keys_pressed; i++) {
            LOG_INF("\t%d: 0x%08x", i, keys_pressed[i]);
        }
        LOG_INF("\n");
    

    }
}

static int init(void)
{
    int rv = 0;
    do
    {
        const struct device* hid_dev = device_get_binding("HID_0");
	    if (hid_dev == NULL) {
	    	LOG_ERR("Cannot get USB HID Device");
	    	rv = -EIO;
            break;
	    }

	    usb_hid_register_device(hid_dev, hid_report_desc, 
            sizeof(hid_report_desc), &ops);
	    usb_hid_init(hid_dev);

        rv = usb_enable(status_cb);
        if (0 != rv) {
            LOG_ERR("Cannot init USB.");
            break;
        }

        k_tid_t tid = k_thread_create(&app_usb_thread_data, app_usb_thread_stack,
                        K_THREAD_STACK_SIZEOF(app_usb_thread_stack), app_usb_thread,
                        NULL, NULL, NULL, CONFIG_APP_USB_THREAD_PRIORITY, 0, 
                        K_MSEC(CONFIG_APP_USB_THREAD_START_DELAY));
        if (NULL == tid) {
            rv = -ENOMEM;
            break;
        }

    } while (false);
    return rv;
}

SYS_INIT_NAMED(APP_USB, init, APPLICATION, 0);

