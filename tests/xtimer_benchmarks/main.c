/*
 * Copyright (C) 2020 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       RIOT timer benchmarks
 *
 * @author      M. Aiman Ismail <muhammadaimanbin.ismail@haw-hamburg.de>

 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "periph/gpio.h"
#include "test_utils/expect.h"
#include "fmt.h"

#include "shell.h"
#include "test_helpers.h"
#include "sc_args.h"
#include "random.h"
#include "timex.h"
#include "mutex.h"

#define ENABLE_DEBUG (0)
#include "debug.h"
#include "log.h"

#ifndef PARSER_DEV_NUM
#define PARSER_DEV_NUM 0
#endif

#define HIL_TEST_REPEAT     (50)
#define HIL_MAX_TIMERS      (50)

#define HIL_TEST_GPIO       GPIO_PIN(TIMER_TRACE_PORT, TIMER_TRACE_PIN)
#define HIL_START_TIMER()   gpio_set(HIL_TEST_GPIO)
#define HIL_STOP_TIMER()    gpio_clear(HIL_TEST_GPIO)
#define HIL_TOGGLE_TIMER()  gpio_toggle(HIL_TEST_GPIO)

#ifndef MODULE_ZTIMER
#include "xtimer.h"

#define TIMER_T                     xtimer_t
#define TIMER_NOW()                 xtimer_now_usec()
#define TIMER_SET(timer, duration)  xtimer_set(timer, duration)
#define TIMER_REMOVE(timer)         xtimer_remove(timer)
#define TIMER_SLEEP(duration)       xtimer_usleep(duration)
#else
#include "ztimer.h"

#define TIMER_T                     ztimer_t
#define ZTIMER_CLOCK                ZTIMER_USEC
#define TIMER_NOW()                 ztimer_now(ZTIMER_CLOCK)
#define TIMER_SET(timer, duration)  ztimer_set(ZTIMER_CLOCK, timer, duration)
#define TIMER_REMOVE(timer)         ztimer_remove(ZTIMER_CLOCK, timer)
#define TIMER_SLEEP(duration)       ztimer_sleep(ZTIMER_CLOCK, duration)
#endif

char printbuf[SHELL_DEFAULT_BUFSIZE] = { 0 };
uint8_t in_buf[64];
uint8_t out_buf[64];

static TIMER_T test_timers[HIL_MAX_TIMERS];

/* Default is whatever, just some small delay if the user forgets to initialize */
static uint32_t spin_max = 64;

/************************
* HELPER FUNCTIONS
************************/

/**
 * @brief   Busy wait (spin) for the given number of loop iterations
 */
static void spin(uint32_t limit)
{
    /* Platform independent busy wait loop, should never be optimized out
     * because of the volatile asm statement */
    while (limit--) {
        __asm__ volatile ("");
    }
}

void spin_random_delay(void)
{
    uint32_t limit = random_uint32_range(0, spin_max);

    spin(limit);
}

/************************
* OVERHEAD
************************/

#define OVERHEAD_SPREAD (1000UL)

void cleanup_overhead(void)
{
    for (unsigned i = 0; i < HIL_MAX_TIMERS; ++i) {
        TIMER_REMOVE(&test_timers[i]);
    }
}

int overhead_gpio_cmd(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    gpio_clear(HIL_TEST_GPIO);

    sprintf(printbuf, "gpio overhead");
    print_cmd(PARSER_DEV_NUM, printbuf);
    for (int i = 0; i < HIL_TEST_REPEAT; i++) {
        HIL_START_TIMER();
        HIL_STOP_TIMER();
        HIL_START_TIMER();
        HIL_STOP_TIMER();

        TIMER_SLEEP(1000);
    }
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int overhead_timer_now(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    sprintf(printbuf, "overhead timer now");
    print_cmd(PARSER_DEV_NUM, printbuf);

    gpio_clear(HIL_TEST_GPIO);

    for (unsigned i = 0; i < HIL_TEST_REPEAT; ++i) {
        spin_random_delay();
        HIL_START_TIMER();
        TIMER_NOW();
        HIL_STOP_TIMER();
    }

    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

static void _overhead_callback(void *arg)
{
    (void)arg;
    expect(false);
}

uint32_t _delay(unsigned n)
{
    return 1 * US_PER_SEC + (OVERHEAD_SPREAD * n);
}

int timer_overhead_timer_cmd(int argc, char **argv)
{
    if (argc < 3) {
        print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
        return -1;
    }

    gpio_clear(HIL_TEST_GPIO);

    const char *method = argv[1];
    const char *pos = argv[2];

    sprintf(printbuf, "timer overhead: %s %s timer", method, pos);
    print_cmd(PARSER_DEV_NUM, printbuf);

    sprintf(printbuf, "%u", HIL_MAX_TIMERS);
    print_data_dict_str(PARSER_DEV_NUM, "timer count", printbuf);

    sprintf(printbuf, "%u", HIL_TEST_REPEAT);
    print_data_dict_str(PARSER_DEV_NUM, "sample count", printbuf);

    /* init the timers */
    for (unsigned i = 0; i < HIL_MAX_TIMERS; ++i) {
        test_timers[i].callback = _overhead_callback;
    }

    unsigned timer_idx = -1;
    if (strcmp(pos, "first") == 0) {
        timer_idx = 0;
    }
    else if (strcmp(pos, "middle") == 0) {
        timer_idx = (HIL_MAX_TIMERS / 2) - 1;
    }
    else if (strcmp(pos, "last") == 0) {
        timer_idx = HIL_MAX_TIMERS - 1;
    }
    else {
        goto error;
    }

    if (strcmp(method, "set") == 0) {
        for (unsigned n = 0; n < HIL_TEST_REPEAT; ++n) {
            /* set all but the last timer */
            for (unsigned i = 0; i < timer_idx; ++i) {
                TIMER_SET(&test_timers[i], _delay(i));
            }

            spin_random_delay();

            /* set the last timer */
            HIL_START_TIMER();
            TIMER_SET(&test_timers[timer_idx], _delay(timer_idx));
            HIL_STOP_TIMER();

            cleanup_overhead();
        }
    }
    else if (strcmp(method, "remove") == 0) {
        for (unsigned n = 0; n < HIL_TEST_REPEAT; ++n) {
            /* set timers until timer_idx */
            for (unsigned i = 0; i <= timer_idx; ++i) {
                TIMER_SET(&test_timers[i], _delay(i));
            }

            spin_random_delay();

            /* remove the timer at timer_idx */
            HIL_START_TIMER();
            TIMER_REMOVE(&test_timers[timer_idx]);
            HIL_STOP_TIMER();

            cleanup_overhead();
        }
    }
    else {
error:
        cleanup_overhead();
        print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
        return -1;
    }

    cleanup_overhead();

    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int timer_overhead_nth_timer_cmd(int argc, char **argv)
{
    if (argc < 3) {
        print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
        return -1;
    }

    gpio_clear(HIL_TEST_GPIO);

    const char *method = argv[1];
    unsigned timer_idx = atoi(argv[2]) - 1;

    cleanup_overhead();

    /* init the timers */
    for (unsigned i = 0; i < HIL_MAX_TIMERS; ++i) {
        test_timers[i].callback = _overhead_callback;
    }

    if (strcmp(method, "set") == 0) {
        for (unsigned n = 0; n < HIL_TEST_REPEAT; ++n) {
            /* set all but the last timer */
            for (unsigned i = 0; i < timer_idx; ++i) {
                TIMER_SET(&test_timers[i], _delay(i));
            }

            spin_random_delay();

            /* set the last timer */
            HIL_START_TIMER();
            TIMER_SET(&test_timers[timer_idx], _delay(timer_idx));
            HIL_STOP_TIMER();

            cleanup_overhead();
        }
    }
    else if (strcmp(method, "remove") == 0) {
        for (unsigned n = 0; n < HIL_TEST_REPEAT; ++n) {
            /* set all timer */
            for (unsigned i = 0; i <= timer_idx; ++i) {
                TIMER_SET(&test_timers[i], _delay(i));
            }

            spin_random_delay();

            /* remove the last timer */
            HIL_START_TIMER();
            TIMER_REMOVE(&test_timers[timer_idx]);
            HIL_STOP_TIMER();

            cleanup_overhead();
        }
    }

    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);

    return 0;
}

/************************
* ACCURACY
************************/

void _sleep_accuracy_timer_set_cb(void *arg)
{
    HIL_STOP_TIMER();
    bool *triggered = (bool *)arg;
    *triggered = true;
}

int sleep_accuracy_timer_set_cmd(int argc, char **argv)
{
    if (argc < 2) {
        print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
        return -1;
    }

    gpio_clear(HIL_TEST_GPIO);

    int sleeptime = strtol(argv[1], NULL, 10);
    if (sleeptime < 0) {
        print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
        return -1;
    }

    sprintf(printbuf, "sleep_accuracy: timer_sleep(%s)", argv[1]);
    print_cmd(PARSER_DEV_NUM, printbuf);

    volatile bool triggered;
    TIMER_T timer = {
        .callback = _sleep_accuracy_timer_set_cb,
        .arg = (void *)&triggered,
    };

    /* measure using PHiLIP */
    for (unsigned i = 0; i < HIL_TEST_REPEAT; i++) {
        spin_random_delay();
        triggered = false;
        HIL_START_TIMER();
        TIMER_SET(&timer, sleeptime);
        while (!triggered) {}
    }

    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int sleep_accuracy_timer_sleep_cmd(int argc, char **argv)
{
    if (argc < 2) {
        print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
        return -1;
    }

    gpio_clear(HIL_TEST_GPIO);

    sprintf(printbuf, "sleep_accuracy: timer_sleep(%s)", argv[1]);
    print_cmd(PARSER_DEV_NUM, printbuf);

    int sleeptime = strtol(argv[1], NULL, 10);
    if (sleeptime < 0) {
        print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
        return -1;
    }

    /* measure using PHiLIP */
    for (unsigned i = 0; i < HIL_TEST_REPEAT; i++) {
        spin_random_delay();
        HIL_START_TIMER();
        TIMER_SLEEP(sleeptime);
        HIL_STOP_TIMER();
    }

    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

/************************
* JITTER
************************/

#define JITTER_TIMER_INTERVAL   (10 * MS_PER_SEC)
#define JITTER_WAKEUPS          (2 * HIL_TEST_REPEAT)
#define JITTER_START_RECORD     5
#define JITTER_PARAM_SIZE       25

typedef struct sleep_jitter_params {
    TIMER_T *timer;
    uint8_t idx;
    uint8_t iter;
    uint8_t recorded;
} jitter_params_t;

static mutex_t jitter_mutex = MUTEX_INIT_LOCKED;
static jitter_params_t jitter_params[JITTER_PARAM_SIZE];
static uint32_t jitter_wakeups[JITTER_WAKEUPS];
static uint32_t jitter_start;
static bool start_record = false;
static bool jitter_end = false;

void cleanup_jitter(unsigned count, jitter_params_t *params)
{
    TIMER_SLEEP(1 * US_PER_SEC);
    for (unsigned i = 0; i < count; ++i) {
        TIMER_REMOVE(params[i].timer);
    }

    memset(jitter_params, 0, sizeof(jitter_params));
    memset(jitter_wakeups, 0, sizeof(jitter_wakeups));
    jitter_start = 0;
    start_record = false;
    jitter_end = false;

    HIL_STOP_TIMER();
}

static uint32_t _next_target(jitter_params_t *params)
{
    return (++params->iter * JITTER_TIMER_INTERVAL) + jitter_start;
}

static void jitter_main_cb(void *arg)
{
    jitter_params_t *params = (jitter_params_t *)arg;

    if (!jitter_end) {
        uint32_t now = TIMER_NOW();
        if (start_record) {
            HIL_TOGGLE_TIMER();
            jitter_wakeups[params->recorded++] = now;
        }
        TIMER_SET(params->timer, _next_target(params) - now);

        if (params->recorded  >= JITTER_WAKEUPS) {
            jitter_end = true;
            start_record = false;
            mutex_unlock(&jitter_mutex);
        }
    }
}

static void jitter_cb(void *arg)
{
    if (!jitter_end) {
        jitter_params_t *params = (jitter_params_t *)arg;
        TIMER_SET(params->timer, _next_target(params) - TIMER_NOW());
    }
}

int sleep_jitter_cmd(int argc, char **argv)
{
    if (argc < 2) {
        print_data_str(PARSER_DEV_NUM, "Not enough arguments");
        print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
        return -1;
    }

    print_cmd(PARSER_DEV_NUM, "sleep_jitter");

    unsigned timer_count = atoi(argv[1]);
    sprintf(printbuf, "%u", timer_count);
    print_data_dict_str(PARSER_DEV_NUM, "timer-count", printbuf);

    sprintf(printbuf, "%lu", JITTER_TIMER_INTERVAL);
    print_data_dict_str(PARSER_DEV_NUM, "timer-interval", printbuf);

    if (timer_count <= 0 || timer_count > ARRAY_SIZE(jitter_params)) {
        print_data_str(PARSER_DEV_NUM, "timer count invalid");
        print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
        return -1;
    }

    jitter_start = TIMER_NOW();
    HIL_START_TIMER();

    /* setup half of the background timers before the periodic timer */
    for (unsigned i = 0; i < timer_count; i++) {
        jitter_params[i].timer = &test_timers[i];
        jitter_params[i].iter = 0;
        jitter_params[i].idx = i;

        TIMER_T *timer = jitter_params[i].timer;
        timer->callback = (i < timer_count - 1) ? jitter_cb : jitter_main_cb;
        timer->arg = &jitter_params[i];
        TIMER_SET(timer, _next_target(&jitter_params[i]) - jitter_start);
    }

    start_record = true;

    mutex_lock(&jitter_mutex);

    /* Print DUT timer values */
    printf(", { \"start-time\": %" PRIu32 "", jitter_start);
    printf(", \"wakeups\": [");
    for (unsigned i = 0; i < JITTER_WAKEUPS; ++i) {
        printf("%" PRIu32 "%s", jitter_wakeups[i],
               (i < JITTER_WAKEUPS - 1) ? "," : "]");
    }
    printf(" }");

    cleanup_jitter(timer_count, jitter_params);

    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

/************************
* DRIFT
************************/

int drift_cmd(int argc, char **argv)
{
    if (argc < 2) {
        print_data_str(PARSER_DEV_NUM, "Not enough arguments");
        print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
        return -1;
    }

    gpio_clear(HIL_TEST_GPIO);

    uint32_t duration = atoi(argv[1]);  /* duration in microseconds */
    sprintf(printbuf, "drift: %" PRIu32 " us", duration);
    print_cmd(PARSER_DEV_NUM, printbuf);

    uint32_t start = TIMER_NOW();
    HIL_START_TIMER();
    TIMER_SLEEP(duration);
    HIL_STOP_TIMER();
    uint32_t diff = TIMER_NOW() - start;

    uint32_t us = diff % US_PER_SEC;
    uint32_t sec = diff / US_PER_SEC;

    sprintf(printbuf, "%" PRIu32 ".%06" PRIu32 "", sec, us);
    print_data_str(PARSER_DEV_NUM, printbuf);

    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

/************************
* ETC
************************/

int cmd_get_timer_version(int argc, char **argv)
{
    (void)argv;
    (void)argc;

#ifdef MODULE_ZTIMER
    sprintf(printbuf, "ztimer");
#else
    sprintf(printbuf, "xtimer");
#endif
    print_data_str(PARSER_DEV_NUM, printbuf);
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int cmd_get_metadata(int argc, char **argv)
{
    (void)argv;
    (void)argc;

    print_data_str(PARSER_DEV_NUM, RIOT_BOARD);
    print_data_str(PARSER_DEV_NUM, RIOT_VERSION);
    print_data_str(PARSER_DEV_NUM, RIOT_APPLICATION);
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);

    return 0;
}

int cmd_gpio_clear(int argc, char **argv)
{
    (void)argv;
    (void)argc;

    gpio_clear(HIL_TEST_GPIO);

    return 0;
}

int cmd_gpio_set(int argc, char **argv)
{
    (void)argv;
    (void)argc;

    gpio_set(HIL_TEST_GPIO);

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "overhead_gpio", "Benchmark the gpio toggling overhead",
      overhead_gpio_cmd },
    { "overhead_timer_now", "timer now overhead",
      overhead_timer_now },
    { "overhead_timer", "timer set/remove overhead",
      timer_overhead_timer_cmd },
    { "overhead_timer_list", "timer nth list overhead",
      timer_overhead_nth_timer_cmd },
    { "sleep_accuracy_timer_sleep", "Sleep for specified time",
      sleep_accuracy_timer_sleep_cmd },
    { "sleep_accuracy_timer_set", "Sleep for specified time",
      sleep_accuracy_timer_set_cmd },
    { "sleep_jitter", "sleep jitter", sleep_jitter_cmd },
    { "drift", "Drift Simple benchmark", drift_cmd },
    { "get_metadata", "Get the metadata of the test firmware",
      cmd_get_metadata },
    { "get_timer_version", "Get timer version", cmd_get_timer_version },
    { "gclear", "clear", cmd_gpio_clear },
    { "gset", "set", cmd_gpio_set },
    { NULL, NULL, NULL }
};

int main(void)
{
    (void)puts("Welcome to RIOT!");

    gpio_init(HIL_TEST_GPIO, GPIO_OUT);
    /* clear initial state */
    gpio_clear(HIL_TEST_GPIO);

    random_init(0);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
