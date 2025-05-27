
#include <stdio.h>
#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>

LOG_MODULE_REGISTER(usb, LOG_LEVEL_INF);


static int init(void)
{
	if (usb_enable(NULL)) {
		LOG_ERR("Cannot enable USB!");
		return -1;
	}
	LOG_INF("USB enabled.");
    return 0;
}


SYS_INIT_NAMED(
    usb,
    init,
    APPLICATION,
    0
);
 