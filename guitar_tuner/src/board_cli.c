#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

/* ============================= */
/*  Subcommand handlers           */
/* ============================= */

static int cmd_board_clocks(const struct shell *sh, size_t argc, char **argv)
{
    shell_print(sh, "Clock information:");
    shell_print(sh, "\tSysClockFreq = %" PRIu32 " Hz", HAL_RCC_GetSysClockFreq());
    shell_print(sh, "\tHCLKFreq = %" PRIu32 " Hz", HAL_RCC_GetHCLKFreq());
    shell_print(sh, "\tPCLK1Freq = %" PRIu32 " Hz", HAL_RCC_GetPCLK1Freq());
    shell_print(sh, "\tPCLK2Freq = %" PRIu32 " Hz", HAL_RCC_GetPCLK2Freq());

    return 0;
}


SHELL_STATIC_SUBCMD_SET_CREATE(sub_board_cmds,
    SHELL_CMD(clocks, NULL, "Show clock configuration", cmd_board_clocks),
    SHELL_SUBCMD_SET_END /* Must be last */
);


SHELL_CMD_REGISTER(board, &sub_board_cmds,
                   "Board-related commands", NULL);
