
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

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_usb, LOG_LEVEL_INF);

static const uint8_t hid_report_desc[] = HID_KEYBOARD_REPORT_DESC();
static enum usb_dc_status_code usb_status;

static K_SEM_DEFINE(ep_write_sem, 0, 1);

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

struct key_press_evt {
    uint32_t key_code;
    bool pressed;
};


K_MEM_SLAB_DEFINE_STATIC(
    app_usb_report_queue_mem, 
    sizeof(struct key_press_evt), 
    CONFIG_APP_USB_REPORT_QUEUE_LEN, 
    4
);
K_FIFO_DEFINE(app_usb_report_queue_handler);

int app_usb_report_key_press(uint32_t code, bool is_pressed, k_timeout_t timeout)
{
    int rv = 0;
    struct key_press_evt* evt = NULL;

    do {
        rv = k_mem_slab_alloc(
            &app_usb_report_queue_mem,
            (void**)&evt,
            timeout
        );
        if (0 != rv) {
            break;
        }

        *evt = (struct key_press_evt){
            .key_code = code,
            .pressed = is_pressed
        };

        k_fifo_put(
            &app_usb_report_queue_handler,
            evt;
        );
    } while (false);

    return rv;
}

static struct key_press_evt* app_usb_get_key_press_evt(k_timeout_t timeout)
{
    struct key_press_evt* evt = k_fifo_get(
        &app_usb_report_queue_handler,
        timeout
    );

    return evt;
}

static void app_usb_release_key_press_evt(struct key_press_evt* evt)
{
    k_mem_slab_free(
        &app_usb_report_queue_mem,
        evt
    );
}


K_THREAD_STACK_DEFINE(app_usb_thread_stack, CONFIG_APP_USB_THREAD_STACK_SIZE);
struct k_thread app_usb_thread_data;

static void app_usb_thread(void*, void*, void*)
{

    while (true) {
        struct key_press_evt* evt = app_usb_get_key_press_evt(K_FOREVER);
        if (NULL == evt) {
            continue;
        }

        LOG_INF("Key press code: %d, is_pressed: %s", evt->key_code, evt->pressed ? "true" : "false");
    
        app_usb_release_key_press_evt(evt);
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

