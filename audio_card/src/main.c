
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
 
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000
 
/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios)
};
#define LEDS_N		ARRAY_SIZE(leds)

int main(void)
{
	int ret;

	for (int i = 0; i < LEDS_N; i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			return 0;
		}

		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
			return 0;
		}
	}
 
 
	int current_led_indx = 0;
	while (1) {

		ret = gpio_pin_toggle_dt(&leds[current_led_indx]);
		if (ret < 0) {
			return 0;
		}
		current_led_indx = (current_led_indx + 1) % LEDS_N;
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
 