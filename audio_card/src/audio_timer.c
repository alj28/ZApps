
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>
#include <zephyr/drivers/counter.h>

LOG_MODULE_REGISTER(audio_timer, LOG_LEVEL_INF);


const struct device *uac2_tim = DEVICE_DT_GET(DT_ALIAS(uac2_tim));


static int init(void)
{
    if (!device_is_ready(uac2_tim)) {
        LOG_ERR("UAC2 timer not ready");
        return -1;
    }
    counter_start(uac2_tim);
    return 0;
}

SYS_INIT_NAMED(
    audio_timer,
    init,
    APPLICATION,
    1
);

int audio_timer_get(uint32_t* ticks)
{
    return counter_get_value(uac2_tim, ticks);
}

int audio_timer_reset(void)
{
    return counter_reset(uac2_tim);
}

