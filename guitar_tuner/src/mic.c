#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2s.h>
//#include <zephyr/audio/dmic_sw_filter.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/led.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include "signal_buffer.h"
#include "mic_filter.h"


#define BLOCK_SIZE 32
#define NUM_BLOCKS 4

#define STACKSIZE 1024
#define PRIORITY 7
#define SLEEP_TIME_MS 100


#define DEBUG_LED_0 DT_NODELABEL(debug_led_0)
#define DEBUG_LED_1 DT_NODELABEL(debug_led_1)

const struct gpio_dt_spec debug_led = GPIO_DT_SPEC_GET(DEBUG_LED_0, gpios);
const struct gpio_dt_spec debug_led_1 = GPIO_DT_SPEC_GET(DEBUG_LED_1, gpios);

K_MEM_SLAB_DEFINE_STATIC(rx_slab, BLOCK_SIZE, NUM_BLOCKS, 4);



static void thread(void *p1, void *p2, void *p3)
{
    const struct device *i2s_dev = DEVICE_DT_GET(DT_NODELABEL(i2s2));


    if (!device_is_ready(i2s_dev)) {
        printk("I2S device not ready\n");
        return;
    }

    gpio_pin_configure_dt(&debug_led, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&debug_led_1, GPIO_OUTPUT_ACTIVE);
    

    // --- Configure I2S RX ---
    struct i2s_config i2s_cfg = {
        .word_size = 32, // dummy for PDM
        .channels = 1, // mono PDM
        .format = (I2S_FMT_DATA_FORMAT_I2S | I2S_FMT_BIT_CLK_INV),
        .frame_clk_freq = 16000, // PDM clock
        .block_size = BLOCK_SIZE,
        .mem_slab = &rx_slab,
        .timeout = 2000,
        .options = (I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER),
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

    mic_filter_init();

    printk("Start I2S loop");
    int32_t output_prev = 0;
    // --- Main loop ---
    while (1) {
        uint32_t *pdm_block;
        size_t size = BLOCK_SIZE;

#if 0
        int ret = i2s_read(i2s_dev, (void*)&pdm_block, &size);
        if (ret == 0) {
            size = (size >> 2);
            int32_t output = 0;
            for (size_t i = 0; i < size; i++)
            {
                uint32_t sample = pdm_block[i];
                uint32_t intermediate_count = 0;
                for (size_t b = 0; b < 32; b++)
                {
                    intermediate_count += (sample & 0x1);
                    sample = sample >> 1;
                }

                int32_t inc = intermediate_count;
                int32_t dec = (32 - intermediate_count) * (-1);

                //output += ((inc + dec) << 7);
                output += (inc + dec);
            }

            //int32_t comb = output - output_prev;
            //output_prev = output;

            //int32_t pcm_sample = comb >> 8;

            push_to_signal_buffer(output);
            gpio_pin_toggle_dt(&debug_led);

            k_mem_slab_free(&rx_slab, pdm_block);
        }
#else
        int ret = i2s_read(i2s_dev, (void*)&pdm_block, &size);
        if (ret == 0) {
            size = (size >> 2);
            int32_t output = 0;
            gpio_pin_set_dt(&debug_led_1, 1);
            for (size_t i = 0; i < size; i++)
            {
                uint32_t sample = pdm_block[i];
                for (size_t b = 0; b < 32; b++)
                {
                    int32_t is_new_sample_available = mic_filter_push(sample & 0x1);
                    if (0 != is_new_sample_available)
                    {
                        push_to_signal_buffer(mic_filter_get_last_output());
                    }
                    sample = sample >> 1;
                }
            }
            gpio_pin_set_dt(&debug_led_1, 0);

            gpio_pin_toggle_dt(&debug_led);

            k_mem_slab_free(&rx_slab, pdm_block);
        }
#endif
    }
}

K_THREAD_DEFINE(mic_thread_id, STACKSIZE, thread, NULL, NULL, NULL,
		PRIORITY, 0, 0);