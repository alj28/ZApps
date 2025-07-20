
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_audio.h>

#include "audio_conf.h"
#include "audio_stream.h"
#include "audio_output_codec.h"
#include "audio_feedback_counter.h"


LOG_MODULE_REGISTER(audio_usb, LOG_LEVEL_INF);

static uint32_t sof_counter_last_val = 0;
static uint32_t average_consumed_samples_per_sof_q14 = 0;

static volatile uint32_t data_received_counter = 0;
static volatile bool print_received_data = false;
static void data_received(const struct device *dev,
			  struct net_buf *buffer,
			  size_t size)
{
	int ret;

	if (!buffer || !size) {
		/* This should never happen */
		return;
	}

#if 0
	LOG_INF("Received %d data, buffer %p", size, buffer);

	/* Check if the device OUT buffer can be used for input */
	if (size == usb_audio_get_in_frame_size(dev)) {
		ret = usb_audio_send(dev, buffer, size);
		if (ret) {
			net_buf_unref(buffer);
		}
	} else {
		net_buf_unref(buffer);
	}
#else
	if (BYTES_PER_SOF != size) {
		net_buf_unref(buffer);
		return;
	}

	if (5000 > k_uptime_get()) {
		net_buf_unref(buffer);
		return;
	}

	if (!is_audio_codec_setup()) {
		net_buf_unref(buffer);
		return;
	}

#define SELECT	2
#if SELECT == 1
	struct audio_chunk* audio_chunk = audio_stream_chunk_alloc(K_NO_WAIT);
	if (NULL != audio_chunk) {
		net_buf_linearize(
			(void*)audio_chunk->mem,
			BYTES_PER_SOF,
			buffer,
			0,
			size
		);
		audio_chunk->size = BYTES_PER_SOF;
		audio_stream_chunk_commit(audio_chunk);
		data_received_counter++;
	}
#elif SELECT == 2
	uint16_t sof_count_state = audio_sof_counter();
	struct audio_chunk* audio_chunk = audio_stream_chunk_alloc(K_NO_WAIT);
	if (NULL != audio_chunk) {
		memcpy(audio_chunk->mem, buffer->data, BYTES_PER_SOF);
		audio_chunk->size = BYTES_PER_SOF;
		audio_stream_chunk_commit(audio_chunk);
	}
	uint16_t sof_count_diff = sof_count_state - sof_counter_last_val;
	sof_counter_last_val = sof_count_state;
	average_consumed_samples_per_sof_q14 = (average_consumed_samples_per_sof_q14 + (sof_count_diff << 14)) >> 1;

#endif

	net_buf_unref(buffer);
#endif
}

static void feature_update(const struct device *dev,
			   const struct usb_audio_fu_evt *evt)
{
	int16_t volume = 0;

	LOG_INF("N received: %d", data_received_counter);
	print_received_data = true;
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

static int init(void)
{
	const struct device *hs_dev;
	int ret;

	LOG_INF("Entered %s", __func__);
	hs_dev = DEVICE_DT_GET_ONE(usb_audio_hs);

	if (!device_is_ready(hs_dev)) {
		LOG_ERR("Device USB Headset is not ready");
		return 0;
	}

	LOG_INF("Found USB Headset Device");

	usb_audio_register(hs_dev, &ops);

	//ret = usb_enable(NULL);
	//if (ret != 0) {
	//	LOG_ERR("Failed to enable USB");
	//	return 0;
	//}
    //
	//LOG_INF("USB enabled");
	return 0;
}

SYS_INIT_NAMED(
    audio_usb_init,
    init,
    APPLICATION,
    50
);


static int cmd_average_consumed_samples_per_sof(const struct shell *sh, size_t argc, char **argv)
{
	uint32_t average_consumed_samples_per_sof_q14_cpy = average_consumed_samples_per_sof_q14;
	float average_consumed_samples_per_sof = ((float)average_consumed_samples_per_sof_q14_cpy / (float)(0x1 << 14));
	shell_print(sh, "Average consumed samples per sof (float): %.08f", average_consumed_samples_per_sof);
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_audio_usb,
    SHELL_CMD(avg_samples_per_sof, NULL, "Average number of samples consumed per SOF.", cmd_average_consumed_samples_per_sof),
    SHELL_SUBCMD_SET_END /* Must be last */
);

SHELL_COND_CMD_REGISTER(CONFIG_AUDIO_USB_SHELL, audio_usb, &sub_audio_usb, "Audio USB commands", NULL);