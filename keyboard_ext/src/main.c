
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
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);



int main(void)
{
    int ret = usb_enable(NULL);
    if (0 != ret) {
        LOG_ERR("Cannot init USB.");
        return 0;
    }

    while (true) {
        k_msleep(500);
    }

    return 0;
}
