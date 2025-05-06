
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

#define HS_NODE DT_NODELABEL(usb_audio_hs)

//static const unsigned int hp_resolution = DT_PROP(HS_NODE, hp_resolution);
//static const unsigned int hp_sample_rate = DT_PROP(HS_NODE, hp_sample_rate_hz);
//static const unsigned int mem_slab_block_size = (int)(2*(hp_resolution/8)*(hp_sample_rate/1000));
 
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
	     "Console device is not ACM CDC UART device");

#define CS43L22_NODE DT_NODELABEL(audio_codec)

const struct device* i2s_dev_codec = NULL;
const struct device* codec_dev = NULL;
 
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

#if 0
K_MEM_SLAB_DEFINE_STATIC(mem_slab, );

static int codec_init(void)
{
	struct audio_codec_cfg audio_cfg = {
		//.mclk_freq = ,	
		.dai_type = AUDIO_DAI_TYPE_I2S,	
		.dai_cfg.i2s = {
			.word_size = 16U,
			.channels = 2,
			.format = I2S_FMT_DATA_FORMAT_I2S,
			.options = I2S_OPT_FRAME_CLK_MASTER,
			.frame_clk_freq = CONFIG_SAMPLE_FREQ,
			.mem_slab = &mem_slab,
			.block_size = ,
			.timeout = 2000,
		},	
		.dai_route = AUDIO_ROUTE_PLAYBACK,	
	};
	if (!audio_codec_configure(codec_dev, &audio_cfg)) {
		LOG_ERR("Cannot configure codec.");
		return -1;
	}
	if (0 > i2s_configure(i2s_dev, I2S_DIR_TX, &audio_cfg->dai_cfg.i2s)) {
		LOG_ERR("Cannot configure i2s.");
		return -1;
	}
	return 0;
}
#endif

struct received_data_mem_slab_fifo_data {
	void *fifo_reversed;
	int16_t samples[16];
};


K_FIFO_DEFINE(received_data_mem_slabs_fifo);

K_MEM_SLAB_DEFINE(
	received_data_mem_slab,
	sizeof(struct received_data_mem_slab_fifo_data),
	5,													// number of blocks
	4
);

void push_received_data_to_queue(struct net_buf *buffer, size_t size)
{
	struct received_data_mem_slab_fifo_data* ptr;
	if (0 == k_mem_slab_alloc(&received_data_mem_slab, (void **)&ptr, K_NO_WAIT)) {
		size_t offset = 0;
		size_t const number_of_samples = size / 2 / 2;
		for (size_t i = 0; (i < number_of_samples) && (offset < 16); i++) {
			ptr->samples[offset++] = net_buf_pull_le16(buffer);
			ptr->samples[offset++] = net_buf_pull_le16(buffer);
		}
		k_fifo_put(&received_data_mem_slabs_fifo, (void*)ptr);
	}
	else
	{
		printf("Receive data memory slab is full.");
	}
}

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
		push_received_data_to_queue(buffer, size);
	} 
	net_buf_unref(buffer);
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

	i2s_dev_codec = DEVICE_DT_GET(DT_ALIAS(i2s_codec_tx));
	codec_dev = DEVICE_DT_GET(DT_NODELABEL(audio_codec));

	if (!device_is_ready(i2s_dev_codec)) {
		LOG_ERR("I2S device is not ready!");
		return 0;
	}
	LOG_INF("I2S device ready.");
	
	if (!device_is_ready(codec_dev)) {
		LOG_ERR("CODEC is not ready!");
		return 0;
	}
	LOG_INF("CODEC device ready.");


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

	const struct device *cs43l22_dev = DEVICE_DT_GET(CS43L22_NODE);
	if (!device_is_ready(cs43l22_dev)) {
		LOG_ERR("Audio codec not ready!");
		return 0;
	}

	LOG_INF("Audio codec ready.");
	//LOG_INF("Audio resolution: %d bits.", hp_resolution);
	//LOG_INF("Audio sampling rate: %d Hz.", hp_sample_rate);

	//if (0 > codec_init()) {
	//	return 0;
	//}

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
 