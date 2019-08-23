/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Test application for periph timer API
 *
 * @author      Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "shell.h"
#include "periph/timer.h"
#include "periph/gpio.h"
#include "mutex.h"

#define ARG_ERROR       (-1)
#define CONVERT_ERROR   (-32768)
#define RESULT_OK       (0)
#define RESULT_ERROR    (-1)
#define INVALID_ARGS    puts("Error: Invalid number of arguments");

#define CB_TOGGLE_STR   "cb_toggle"
#define CB_HIGH_STR     "cb_high"
#define CB_LOW_STR      "cb_low"

static mutex_t cb_mutex;

static inline int _get_num(const char *str)
{
    errno = 0;
    char *temp;
    long val = strtol(str, &temp, 0);

    if (temp == str || *temp != '\0' ||
        ((val == LONG_MIN || val == LONG_MAX) && errno == ERANGE)) {
        val = CONVERT_ERROR;
    }
    return (int)val;
}

static int _check_param(int argc, char **argv, int c_min, int c_max, char *use)
{
    int dev;

    if (argc - 1 < c_min || argc - 1 > c_max) {
        printf("Usage: %s %s\n", argv[0], use);
        INVALID_ARGS;
        return ARG_ERROR;
    }

    dev = _get_num(argv[1]);
    if (dev < 0 || dev >= (int)TIMER_NUMOF) {
        printf("Error: No device, only %d supported\n", (int)TIMER_NUMOF);
        return ARG_ERROR;
    }
    return dev;
}

static int _print_cmd_result(const char *cmd, bool success, int ret)
{
    printf("%s: %s(): [%d]\n", success ? "Success" : "Error", cmd, ret);
    return success ? RESULT_OK : RESULT_ERROR;
}

static gpio_t _get_pin(const char *port_str, const char *pin_str)
{
    int port = _get_num(port_str);
    int pin = _get_num(pin_str);

    return GPIO_PIN(port, pin);
}

void cb_toggle(void *arg, int channel)
{
    (void)channel;
    gpio_t pin = (gpio_t)(intptr_t)arg;
    gpio_toggle(pin);
    mutex_unlock(&cb_mutex);
}

void cb_high(void *arg, int channel)
{
    (void)channel;
    gpio_t pin = (gpio_t)(intptr_t)arg;
    gpio_set(pin);
    mutex_unlock(&cb_mutex);
}

void cb_low(void *arg, int channel)
{
    (void)channel;
    gpio_t pin = (gpio_t)(intptr_t)arg;
    gpio_clear(pin);
    mutex_unlock(&cb_mutex);
}

int cmd_timer_bench_read(int argc, char **argv)
{
    int timer_dev = _check_param(argc, argv, 4, 4,
                                 "timerdev repeat_cnt gpio_port gpio_pin");

    if (timer_dev == ARG_ERROR) {
        return -1;
    }

    uint32_t repeat_cnt = _get_num(argv[2]);

    gpio_t pin = _get_pin(argv[3], argv[4]);

    gpio_toggle(pin);

    for (uint32_t i = 0; i < repeat_cnt; i++) {
        timer_read(timer_dev);
    }

    gpio_toggle(pin);

    printf("Success: cmd_timer_read_bench()\n");
    return RESULT_OK;
}

int cmd_timer_init(int argc, char **argv)
{
    int timer_dev = _check_param(argc, argv, 5, 5,
                                 "dev freq cb gpio_port gpio_pin");

    if (timer_dev == ARG_ERROR) {
        return -1;
    }

    unsigned long freq = _get_num(argv[2]);

    timer_cb_t cb = NULL;

    if (strncmp(CB_TOGGLE_STR, argv[3], strlen(argv[3])) == 0) {
        cb = cb_toggle;
    }

    if (strncmp(CB_HIGH_STR, argv[3], strlen(argv[3])) == 0) {
        cb = cb_high;
    }

    if (strncmp(CB_LOW_STR, argv[3], strlen(argv[3])) == 0) {
        cb = cb_low;
    }

    if(cb == NULL) {
        printf("no valid callback name given. Valid values or %s, %s or %s\n",
               CB_TOGGLE_STR, CB_HIGH_STR, CB_LOW_STR);
        return -2;
    }

    gpio_t pin = _get_pin(argv[4], argv[5]);

    gpio_init(pin, GPIO_OUT);

    int res = timer_init(timer_dev, freq, cb, (void*)(intptr_t)pin);

    return _print_cmd_result("timer_init", res == 0, res);
}

int _timer_set(int argc, char **argv, bool absolute)
{
    int res;
    int timer_dev = _check_param(argc, argv, 5, 5,
                                 "dev channel ticks gpio_port gpio_pin");

    if (timer_dev == ARG_ERROR) {
        return -1;
    }

    unsigned long chan = _get_num(argv[2]);
    unsigned long timeout = _get_num(argv[3]);
    gpio_t pin = _get_pin(argv[4], argv[5]);

    mutex_lock(&cb_mutex);

    if (absolute) {
        gpio_toggle(pin);
        res = timer_set_absolute(timer_dev, chan, timeout);
    }
    else {
        gpio_toggle(pin);
        res = timer_set(timer_dev, chan, timeout);
    }

    /* wait for unlock by cb */
    mutex_lock(&cb_mutex);

    /* reset mutex state */
    mutex_unlock(&cb_mutex);
    return res;
}

int cmd_timer_set(int argc, char **argv)
{
    int res = _timer_set(argc, argv, false);
    return _print_cmd_result("timer_set", res == 1, res);
}

int cmd_timer_absolute(int argc, char **argv)
{
    int res = _timer_set(argc, argv, true);
    return _print_cmd_result("timer_set_absolute", res == 1, res);
}

int cmd_timer_clear(int argc, char **argv)
{
    int timer_dev = _check_param(argc, argv, 2, 2, "dev channel");

    if (timer_dev == ARG_ERROR) {
        return -1;
    }

    unsigned long chan = _get_num(argv[2]);

    int res = timer_clear(timer_dev, chan);

    return _print_cmd_result("timer_clear", res == 1, res);
}

int cmd_timer_read(int argc, char **argv)
{
    int timer_dev = _check_param(argc, argv, 1, 1, "dev");

    if (timer_dev == ARG_ERROR) {
        return -1;
    }

    int res = timer_read(timer_dev);
    return _print_cmd_result("timer_read", true, res);
}

int cmd_timer_start(int argc, char **argv)
{
    int timer_dev = _check_param(argc, argv, 1, 1, "dev");

    if (timer_dev == ARG_ERROR) {
        return -1;
    }

    timer_start(timer_dev);
    puts("Success: timer_start()\n");
    return 0;
}

int cmd_timer_stop(int argc, char **argv)
{
    int timer_dev = _check_param(argc, argv, 1, 1, "dev");

    if (timer_dev == ARG_ERROR) {
        return -1;
    }

    timer_stop(timer_dev);
    puts("Success: timer_stop()\n");
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
    { "timer_read_bench", "execute multiple reads to determine overhead",
      cmd_timer_bench_read },
    { "timer_init", "init_timer", cmd_timer_init },
    { "timer_set", "set timer to relative value", cmd_timer_set },
    { "timer_absolute", "set timer to absolute value", cmd_timer_absolute },
    { "timer_clear", "clear timer", cmd_timer_clear },
    { "timer_read", "read timer", cmd_timer_read },
    { "timer_start", "start timer", cmd_timer_start },
    { "timer_stop", "stop timer", cmd_timer_stop },
    { "get_metadata", "Get the metadata of the test firmware",
      cmd_get_metadata },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("Start: Test for the timer API");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
