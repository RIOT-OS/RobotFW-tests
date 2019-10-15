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
#define INVALID_ARGS    puts("Error: Invalid number of arguments")
#define PARSE_ERROR     puts("Error: unable to parse arguments")

#define CB_TOGGLE_STR   "cb_toggle"
#define CB_HIGH_STR     "cb_high"
#define CB_LOW_STR      "cb_low"

static mutex_t cb_mutex;
static gpio_t debug_pins[TIMER_NUMOF];

static inline int _get_num(const char *str, uint32_t* val)
{
    errno = 0;
    char *temp;
    long v = strtol(str, &temp, 0);

    if (temp == str || *temp != '\0' || (v < 0) ||
        ((v == LONG_MIN || v == LONG_MAX) && errno == ERANGE)) {
        return CONVERT_ERROR;
    }

    *val = (uint32_t)v;
    return RESULT_OK;
}

static int _check_param(int argc, char **argv, uint32_t *dev, int c_min,
                        int c_max, char *use)
{
    if (argc - 1 < c_min || argc - 1 > c_max) {
        printf("Usage: %s %s\n", argv[0], use);
        INVALID_ARGS;
        return ARG_ERROR;
    }

    if ((_get_num(argv[1], dev) != RESULT_OK) || *dev >= TIMER_NUMOF) {
        printf("Error: No device, only %u supported\n", TIMER_NUMOF);
        return ARG_ERROR;
    }

    return RESULT_OK;
}

static int _print_cmd_result(const char *cmd, bool success, int ret,
                             bool print_ret)
{
    printf("%s: %s()", success ? "Success" : "Error", cmd);

    if (print_ret) {
        printf(": [%d]", ret);
    }

    printf("\n");

    return success ? RESULT_OK : RESULT_ERROR;
}

static gpio_t _get_pin(const char *port_str, const char *pin_str)
{
    uint32_t port, pin = 0;

    if (_get_num(port_str, &port) == CONVERT_ERROR ||
        _get_num(pin_str, &pin) == CONVERT_ERROR) {
        return GPIO_UNDEF;
    }

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

/* API calls */

int cmd_timer_init(int argc, char **argv)
{
    uint32_t dev = 0;
    uint32_t freq = 0;

    if (_check_param(argc, argv, &dev, 3, 3, "dev freq cb") != RESULT_OK) {
        return RESULT_ERROR;
    }

    if (_get_num(argv[2], &freq) !=  RESULT_OK) {
        return -2;
    }

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
        return -3;
    }

    int res = timer_init(dev, freq, cb, (void*)(intptr_t)debug_pins[dev]);

    return _print_cmd_result("timer_init", res == 0, res, true);
}

int _timer_set(int argc, char **argv, bool absolute)
{
    int res;
    uint32_t dev = 0;

    if (_check_param(argc, argv, &dev, 3, 3,
                     "dev channel ticks") != RESULT_OK) {
        return -1;
    }

    uint32_t chan, timeout = 0;

    if (_get_num(argv[2], &chan) != RESULT_OK ||
        _get_num(argv[3], &timeout) != RESULT_OK) {
        PARSE_ERROR;
    }

    gpio_t pin = debug_pins[dev];

    mutex_lock(&cb_mutex);

    gpio_toggle(pin);
    if (absolute) {
        res = timer_set_absolute(dev, chan, timeout);
    }
    else {
        res = timer_set(dev, chan, timeout);
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
    return _print_cmd_result("timer_set", (res == 0), res, true);
}

int cmd_timer_set_absolute(int argc, char **argv)
{
    int res = _timer_set(argc, argv, true);
    return _print_cmd_result("timer_set_absolute", (res == 0), res, true);
}

int cmd_timer_clear(int argc, char **argv)
{
    uint32_t dev = 0;
    uint32_t chan = 0;

    if (_check_param(argc, argv, &dev, 2, 2, "dev channel") != RESULT_OK) {
        return RESULT_ERROR;
    }

    if (_get_num(argv[2], &chan) != RESULT_OK) {
        PARSE_ERROR;
    }

    int res = timer_clear(dev, chan);

    return _print_cmd_result("timer_clear", (res == 0), res, true);
}

int cmd_timer_read(int argc, char **argv)
{
    uint32_t dev = 0;

    if (_check_param(argc, argv, &dev, 1, 1, "dev") != RESULT_OK) {
        return RESULT_ERROR;
    }

    printf("Success: timer_read(): [%u]\n", timer_read(dev));
    return RESULT_OK;
}

int cmd_timer_start(int argc, char **argv)
{
    uint32_t dev = 0;

    if (_check_param(argc, argv, &dev, 1, 1, "dev") != RESULT_OK) {
        return RESULT_ERROR;
    }

    timer_start(dev);
    return _print_cmd_result("timer_start", true, 0, false);
}

int cmd_timer_stop(int argc, char **argv)
{
    uint32_t dev = 0;

    if (_check_param(argc, argv, &dev, 1, 1, "dev") != RESULT_OK) {
        return RESULT_ERROR;
    }

    timer_stop(dev);
    return _print_cmd_result("timer_stop", true, 0, false);
}

/* helper calls (non-API) */

int cmd_timer_debug_pin(int argc, char **argv)
{
    uint32_t dev = 0;

    if (_check_param(argc, argv, &dev, 3, 3, "dev gpio_port gpio_pin") != RESULT_OK) {
        return RESULT_ERROR;
    }
    /* parse and init debug pin */
    gpio_t pin = _get_pin(argv[2], argv[3]);
    debug_pins[dev] = pin;
    gpio_init(pin, GPIO_OUT);

    return _print_cmd_result("timer_debug_pin", true, 0, false);
}

int cmd_timer_bench_read(int argc, char **argv)
{
    uint32_t dev = 0;
    uint32_t repeat_cnt = 0;

    if (_check_param(argc, argv, &dev, 2, 2, "dev repeat_cnt") != RESULT_OK) {
        return RESULT_ERROR;
    }

    gpio_t pin = debug_pins[dev];

    gpio_toggle(pin);

    for (uint32_t i = 0; i < repeat_cnt; i++) {
        timer_read(dev);
    }

    gpio_toggle(pin);

    return _print_cmd_result("cmd_timer_read_bench", true, 0, false);
}

int cmd_get_metadata(int argc, char **argv)
{
    (void)argv;
    (void)argc;

    printf("Success: [%s, %s]\n", RIOT_BOARD, RIOT_APPLICATION);

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "timer_init", "Initialize timer device", cmd_timer_init },
    { "timer_set", "set timer to relative value", cmd_timer_set },
    { "timer_set_absolute", "set timer to absolute value",
      cmd_timer_set_absolute },
    { "timer_clear", "clear timer", cmd_timer_clear },
    { "timer_read", "read timer", cmd_timer_read },
    { "timer_start", "start timer", cmd_timer_start },
    { "timer_stop", "stop timer", cmd_timer_stop },
    { "timer_debug_pin", "config debug pin", cmd_timer_debug_pin },
    { "timer_read_bench", "execute multiple reads to determine overhead",
      cmd_timer_bench_read },
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
