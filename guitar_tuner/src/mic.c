#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2s.h>
//#include <zephyr/audio/dmic_sw_filter.h>
#include <zephyr/sys/printk.h>


#define BLOCK_SIZE 256
#define NUM_BLOCKS 4

#define STACKSIZE 1024
#define PRIORITY 7
#define SLEEP_TIME_MS 100


K_MEM_SLAB_DEFINE_STATIC(rx_slab, BLOCK_SIZE, NUM_BLOCKS, 4);



static void thread(void *p1, void *p2, void *p3)
{
    const struct device *i2s_dev = DEVICE_DT_GET(DT_NODELABEL(i2s2));


    if (!device_is_ready(i2s_dev)) {
        printk("I2S device not ready\n");
        return;
    }


    // --- Configure I2S RX ---
    struct i2s_config i2s_cfg = {
        .word_size = 32, // dummy for PDM
        .channels = 1, // mono PDM
        .format = I2S_FMT_DATA_FORMAT_I2S,
        .frame_clk_freq = 16000, // PDM clock
        .block_size = BLOCK_SIZE,
        .mem_slab = &rx_slab,
        .timeout = 2000,
        .options = (I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER),
        //.options = (I2S_OPT_BIT_CLK_MASTER),
        //.options = (I2S_OPT_FRAME_CLK_MASTER),
    };


    int rv = i2s_configure(i2s_dev, I2S_DIR_RX, &i2s_cfg);
    if (0 != rv) {
        printk("I2S configuration failed.");
        return;
    }

    rv = i2s_trigger(i2s_dev, I2S_DIR_RX, I2S_TRIGGER_START);
    if (0 != rv) {
        printk("I2S trigger failed.");
        return;
    }

    printk("Start I2S loop");
    // --- Main loop ---
    while (1) {
        void *pdm_block;
        size_t size = BLOCK_SIZE;


        int ret = i2s_read(i2s_dev, &pdm_block, &size);
        if (ret == 0) {
            //int16_t pcm[64]; // depends on decimation factor
            //int pcm_samples = dmic_sw_filter_process(&filter, pdm_block, size, pcm);
            //
            //
            //if (pcm_samples > 0) {
            //    process_audio_samples(pcm, pcm_samples);
            //}
            k_mem_slab_free(&rx_slab, pdm_block);
        }
    }
}

K_THREAD_DEFINE(mic_thread_id, STACKSIZE, thread, NULL, NULL, NULL,
		PRIORITY, 0, 0);