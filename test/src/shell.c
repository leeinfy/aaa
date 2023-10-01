#include <zephyr.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

static int cmd_demo_ping(const struct shell *sh, size_t argc, char **argv){
    printk("pong\n");
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_demo,
    SHELL_CMD(ping, NULL, "PING command", cmd_demo_ping),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(demo, &sub_demo, "DEMO COMMANDS", NULL);

void main(void){
    printk("init\n");
}