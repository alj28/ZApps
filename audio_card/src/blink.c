
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

LOG_MODULE_REGISTER(blink, LOG_LEVEL_INF);

#define STACKSIZE 1024
#define PRIORITY 7
#define SLEEP_TIME_MS 1000

static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios)
};
#define LEDS_N		ARRAY_SIZE(leds)

void blink_thread(void) 
{
	for (size_t i = 0; i < LEDS_N; i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			return;
		}

		int ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
			return;
		}
	}

	int current_led_indx = 0;
	for(;;) {
		int ret = gpio_pin_toggle_dt(&leds[current_led_indx]);
		if (ret < 0) {
			LOG_ERR("Cannot toggle LED.");
			return;
		}
		current_led_indx = (current_led_indx + 1) % LEDS_N;
		k_msleep(SLEEP_TIME_MS);

	}
}


K_THREAD_DEFINE(blink_thread_id, STACKSIZE, blink_thread, NULL, NULL, NULL,
		PRIORITY, 0, 0);
 