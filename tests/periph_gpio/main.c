/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for GPIO peripheral drivers
 *
 * @author      Kevin Weiss <kevin.weiss@haw-hamuburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>

#include "shell.h"
#include "periph/gpio.h"


static int set(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s <port> <pin>\n", argv[0]);
        return 1;
    }
    gpio_init(GPIO_PIN(atoi(argv[1]), atoi(argv[2])), GPIO_OUT);
    gpio_set(GPIO_PIN(atoi(argv[1]), atoi(argv[2])));
    printf("Success: Pin set\n");
    return 0;
}

static int clear(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s <port> <pin>\n", argv[0]);
        return 1;
    }
    gpio_init(GPIO_PIN(atoi(argv[1]), atoi(argv[2])), GPIO_OUT);
    gpio_clear(GPIO_PIN(atoi(argv[1]), atoi(argv[2])));
    printf("Success: Pin cleared\n");
    return 0;
}

int cmd_get_metadata(int argc, char **argv)
{
    (void)argv;
    (void)argc;

    printf("Success: [%s, %s]\n", RIOT_BOARD, RIOT_APPLICATION);

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "gpio_set", "set pin to HIGH", set },
    { "gpio_clear", "set pin to LOW", clear },
    { "get_metadata", "Get the metadata of the test firmware", cmd_get_metadata },
    { NULL, NULL, NULL }
};

int main(void)
{
    /* start the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
