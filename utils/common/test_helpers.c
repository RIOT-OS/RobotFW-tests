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
 * @brief       Test helpers for writing tests with standard interfaces.
 *
 * @author      Kevin Weiss <kevin.weiss@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "test_helpers.h"

#ifdef JSON_SHELL_PARSER
static int parser_state[NUM_OF_JSON_SHELL_PARSER] = {0};
#endif

#ifdef JSON_SHELL_PARSER
static void _start_json(int dev) {
    if (parser_state[dev] == JSON_STATE_READY) {
        printf("{");
        parser_state[dev] |= JSON_STATE_STARTED;
    }
    else {
        printf(",");
    }
}
#endif

void print_cmd(int dev, char *cmd)
{
#ifdef JSON_SHELL_PARSER
    _start_json(dev);
    assert(!(parser_state[dev] & JSON_STATE_DATA_STARTED));
    printf("\"cmd\":\"%s\"", cmd);
#else
    (void)dev;
    puts(cmd);
#endif
}

void print_data_dict_str(int dev, char *key, char *val)
{
#ifdef JSON_SHELL_PARSER
    _start_json(dev);
    if (!(parser_state[dev] & JSON_STATE_DATA_STARTED))
    {
        printf("\"data\":[");
        parser_state[dev] |= JSON_STATE_DATA_STARTED;
    }
    printf("{\"%s\":\"%s\"}", key, val);
#else
    (void)dev;
    printf("%s: %s\n", key, val);
#endif
}

void print_data_int(int dev, int32_t data)
{
#ifdef JSON_SHELL_PARSER
    _start_json(dev);
    if (!(parser_state[dev] & JSON_STATE_DATA_STARTED))
    {
        printf("\"data\":[");
        parser_state[dev] |= JSON_STATE_DATA_STARTED;
    }
    printf("%" PRIi32, data);
#else
    (void)dev;
    printf("%" PRIi32 "\n", data);
#endif
}

void print_data_str(int dev, char *str)
{
#ifdef JSON_SHELL_PARSER
    _start_json(dev);
    if (!(parser_state[dev] & JSON_STATE_DATA_STARTED))
    {
        printf("\"data\":[");
        parser_state[dev] |= JSON_STATE_DATA_STARTED;
    }
    printf("\"%s\"", str);
#else
    (void)dev;
    puts(str);
#endif
}

void print_result(int dev, char *res)
{
#ifdef JSON_SHELL_PARSER
    if ((parser_state[dev] & JSON_STATE_DATA_STARTED))
    {
        printf("]");
        parser_state[dev] &= ~JSON_STATE_DATA_STARTED;
    }
    _start_json(dev);
    printf("\"result\":\"%s\"}\n", res);
    parser_state[dev] &= ~JSON_STATE_STARTED;
#else
    (void)dev;
    puts(res);
#endif
}
