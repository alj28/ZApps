
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/shell/shell.h>
#include <zephyr/init.h>

LOG_MODULE_REGISTER(board_shell, LOG_LEVEL_INF);

#if defined(CONFIG_BOARD_STM32F4_DISCO)

static int cmd_clocks(const struct shell *sh, size_t argc, char **argv)
{
    float sysclk = (float)(HAL_RCC_GetSysClockFreq())/1000000.0f;
    float hclk = (float)(HAL_RCC_GetHCLKFreq())/1000000.0f;
    float pclk1 = (float)(HAL_RCC_GetPCLK1Freq())/1000000.0f;
    float pclk2 = (float)(HAL_RCC_GetPCLK2Freq())/1000000.0f;

    shell_print(sh, "SYSCLK:  %.02f MHz", (double)(sysclk));
    shell_print(sh, "HCLK:    %.02f MHz", (double)(hclk));
    shell_print(sh, "PCLK1:   %.02f MHz", (double)(pclk1));
    shell_print(sh, "PCLK2:   %.02f MHz", (double)(pclk2));
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_stm32f4_disco,
    SHELL_CMD(clocks, NULL, "Internal buses clock frequencies.", cmd_clocks),
    SHELL_SUBCMD_SET_END /* Must be last */
);

SHELL_CMD_REGISTER(stm32f4_disco, &sub_stm32f4_disco, "STM32F4 Discovery commands", NULL);

#endif


static int init(void)
{
#if defined(CONFIG_BOARD_STM32F4_DISCO)
    const char* board_name = "STM32F4_DISCO";
#endif

    LOG_INF("Board: %s", board_name);
    return 0;
}

SYS_INIT_NAMED(
    board_shell,
    init,
    APPLICATION,
    1
);