
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/shell/shell.h>
#include <zephyr/init.h>


static int cmd_reboot(const struct shell *sh, size_t argc, char **argv)
{
    shell_print(sh, "Rebooting the device...");
    k_sleep(K_MSEC(1000)); 
    sys_reboot(SYS_REBOOT_COLD);
    return 0;
}

SHELL_CMD_REGISTER(reboot, NULL, "Reboot the device.", cmd_reboot);