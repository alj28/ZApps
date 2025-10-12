
#include <errno.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include "app_usb.h"
#include "app_keyboard.h"
#include "config.h"

static int key_simulation_argv_parser(const struct shell* sh, size_t argc, char** argv, uint32_t* key_code) 
{
    int rv = 0;

    do {
        if (argc < 2) {
            shell_error(sh, "Usage: press <key code>");
            rv = -EINVAL;
            break;
        }

        *key_code = 0;

        char *endptr = NULL;
        errno = 0;
        size_t argv_len = strlen(argv[1]);
        uint32_t key_code_local = strtol(argv[1], &endptr, 10);

        if (0 != errno) {
            shell_error(sh, "Invalid string.");
            rv = errno;
            break;
        } 
        if (endptr == argv[1]) {
            shell_error(sh, "No digits found.");
            rv = -EINVAL;
            break;
        }
        if ((argv[1] + argv_len) != endptr) {
            shell_error(sh, "Not whole string processed");
            rv = -EINVAL;
            break;
        }
        
        *key_code = key_code_local;
    } while(false);

    return rv;
}

static int cmd_press(const struct shell *sh, size_t argc, char **argv)
{
    uint32_t key_code;
    int rv = 0;

    do
    {
        rv = key_simulation_argv_parser(sh, argc, argv, &key_code);
        if (0 != rv) {
            break;
        }

     	rv = app_keyboard_report_key_press(key_code, true, K_MSEC(500));  
        app_usb_report_key_press();
    } while(false);

    return rv;
}

static int cmd_release(const struct shell *sh, size_t argc, char **argv)
{
    uint32_t key_code;
    int rv = 0;

    do
    {
        rv = key_simulation_argv_parser(sh, argc, argv, &key_code);
        if (0 != rv) {
            break;
        }

	rv = app_keyboard_report_key_press(key_code, false, K_MSEC(500));        
	app_usb_report_key_press();
    } while(false);

    return rv;
}

static int cmd_click(const struct shell *sh, size_t argc, char **argv)
{
    uint32_t key_code;
    int rv = 0;

    do
    {
        rv = key_simulation_argv_parser(sh, argc, argv, &key_code);
        if (0 != rv) {
            break;
        }

	rv = app_keyboard_report_key_press(key_code, true, K_MSEC(500));        
	app_usb_report_key_press();
        if (0 != rv) {
            break;
        }

        k_sleep(K_MSEC(CONFIG_APP_CLI_CLICK_TIME));

	rv = app_keyboard_report_key_press(key_code, false, K_MSEC(500));        
	app_usb_report_key_press();
        if (0 != rv) {
            break;
        }
    } while(false);

    return rv;
}

static int cmd_print_descriptor(const struct shell *sh, size_t argc, char **argv)
{
    app_usb_print_descriptor();
    return 0;
}


static int cmd_play_pause(const struct shell *sh, size_t argc, char **argv)
{
    app_usb_play_pause();
    return 0;
}

static int cmd_volume_up(const struct shell *sh, size_t argc, char **argv)
{
    app_usb_volume_up();
    return 0;
}

static int cmd_volume_down(const struct shell *sh, size_t argc, char **argv)
{
    app_usb_volume_down();
    return 0;
}

static int cmd_mute(const struct shell *sh, size_t argc, char **argv)
{
    app_usb_mute();
    return 0;
}



SHELL_STATIC_SUBCMD_SET_CREATE(sub_app,
        SHELL_CMD_ARG(press, NULL, "Simulate press.", cmd_press, 1, 1),
        SHELL_CMD_ARG(release, NULL, "Simulate release", cmd_release, 1, 1),
        SHELL_CMD_ARG(click, NULL, "Simulate click", cmd_click, 1, 1),
        SHELL_CMD(descriptor, NULL, "USB descriptor", cmd_print_descriptor),
        SHELL_CMD(play_pause, NULL, "USB descriptor", cmd_play_pause),
        SHELL_CMD(volume_up, NULL, "USB descriptor", cmd_volume_up),
        SHELL_CMD(volume_down, NULL, "USB descriptor", cmd_volume_down),
        SHELL_CMD(mute, NULL, "USB descriptor", cmd_mute),
        SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(app, &sub_app, "Application commands", NULL);
