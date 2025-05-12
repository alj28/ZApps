
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include <zephyr/audio/codec.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_audio.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);



int main(void)
{
	if (usb_enable(NULL)) {
		LOG_ERR("Cannot enable USB!");
		return 0;
	}
	LOG_INF("USB enabled.");

	while(1) {
		
	}
}
 