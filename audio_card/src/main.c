
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_audio.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);
 
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
	     "Console device is not ACM CDC UART device");
 
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

static void data_received(const struct device *dev,
			  struct net_buf *buffer,
			  size_t size)
{
	int ret;

	if (!buffer || !size) {
		/* This should never happen */
		return;
	}

	LOG_DBG("Received %d data, buffer %p", size, buffer);

	/* Check if the device OUT buffer can be used for input */
	if (size == usb_audio_get_in_frame_size(dev)) {
		ret = usb_audio_send(dev, buffer, size);
		if (ret) {
			net_buf_unref(buffer);
		}
	} else {
		net_buf_unref(buffer);
	}
}

static void feature_update(const struct device *dev,
			   const struct usb_audio_fu_evt *evt)
{
	int16_t volume = 0;

	LOG_DBG("Control selector %d for channel %d updated",
		evt->cs, evt->channel);
	switch (evt->cs) {
	case USB_AUDIO_FU_MUTE_CONTROL:
		break;
	case USB_AUDIO_FU_VOLUME_CONTROL:
		volume = UNALIGNED_GET((int16_t *)evt->val);
		LOG_INF("set volume: %d", volume);
		break;
	default:
		break;
	}
}

static const struct usb_audio_ops ops = {
	.data_received_cb = data_received,
	.feature_update_cb = feature_update,
};

int main(void)
{
	int ret;
	const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t dtr = 0;



	const struct device *hs_dev;
	hs_dev = DEVICE_DT_GET_ONE(usb_audio_hs);

	if (!device_is_ready(hs_dev)) {
		LOG_ERR("Device USB is not ready!");
		return 0;
	}

	usb_audio_register(hs_dev, &ops);

	if (usb_enable(NULL)) {
		LOG_ERR("Cannot enable USB!");
		return 0;
	}

	LOG_INF("USB enabled.");

	/* Poll if the DTR flag was set */
	while (!dtr) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		/* Give CPU resources to low priority threads. */
		k_sleep(K_MSEC(100));
	}

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
 