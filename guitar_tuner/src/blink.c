
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(blink, LOG_LEVEL_INF);
#define STACKSIZE 1024
#define PRIORITY 7
#define SLEEP_TIME_MS 100

enum leds {
    LED_GREEN = 0,
    LED_ORANGE,
    LED_RED,
    LED_BLUE,
    LED_LAST
};

static const struct gpio_dt_spec leds[LED_LAST] = {
	[LED_GREEN] = GPIO_DT_SPEC_GET(DT_ALIAS(ledg), gpios),
	[LED_ORANGE] = GPIO_DT_SPEC_GET(DT_ALIAS(ledo), gpios),
	[LED_RED] = GPIO_DT_SPEC_GET(DT_ALIAS(ledr), gpios),
	[LED_BLUE] = GPIO_DT_SPEC_GET(DT_ALIAS(ledb), gpios)
};

int init(void) 
{
    int rv = 0;

    for (size_t i = 0; i < LED_LAST; i++) {
        if (!gpio_is_ready_dt(&leds[i])) {       // Fix readiness check
            LOG_ERR("Led (index %d) not ready.", i);
            return -ENODEV;
        }

        rv = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
        if (rv < 0) {
            LOG_ERR("Cannot configure led (index %d).", i);
            return rv;
        }
    }

    return rv;
}

static void thread(void *p1, void *p2, void *p3)
{
    int ret = init();
    if (ret < 0) {
        return;
    }

	int current_led_indx = 0;
	while(1) {
		ret = gpio_pin_toggle_dt(&leds[current_led_indx]);
		if (ret < 0) {
			LOG_ERR("Cannot toggle LED.");
			return;
		}
		current_led_indx = (current_led_indx + 1) % LED_LAST;
		k_msleep(SLEEP_TIME_MS);
	}
}

K_THREAD_DEFINE(blink_thread_id, STACKSIZE, thread, NULL, NULL, NULL,
		PRIORITY, 0, 0);
