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
 * @brief       Shell helpers
 *
 * @author      Sebastian Meiling <s@mlng.net>
 *
 * @}
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "shell.h"
#include "sc_args.h"

#define INVALID_ARGS    puts("Error: Invalid number of arguments");

int sc_args_check(int argc, char **argv, int c_min, int c_max, char *use)
{
    if (argc - 1 < c_min || argc - 1 > c_max) {
        printf("Usage: %s %s\n", argv[0], use);
        INVALID_ARGS;
        return ARGS_ERROR;
    }
    return ARGS_OK;
}

int sc_arg2long(const char *arg, long *val)
{
    errno = 0;
    char *end;
    long res = strtol(arg, &end, 0);

    if ((*end != '\0') || ((res == LONG_MIN || res == LONG_MAX) && errno == ERANGE)) {
        return ARGS_ERROR;
    }
    *val = res;
    return ARGS_OK;
}

int sc_arg2int(const char *arg, int *val)
{
    long lval;
    int res = sc_arg2long(arg, &lval);
    if (res == ARGS_OK) {
        *val = (int)lval;
    }
    return res;
}

int sc_arg2ulong(const char *arg, unsigned long *val)
{
    errno = 0;
    char *end;
    unsigned long res = strtoul(arg, &end, 0);

    if ((*end != '\0') || (res == LONG_MAX && errno == ERANGE)) {
        return ARGS_ERROR;
    }
    *val = res;
    return ARGS_OK;
}

int sc_arg2uint(const char *arg, unsigned int *val)
{
    unsigned long lval;
    int res = sc_arg2ulong(arg, &lval);
    if (res == ARGS_OK) {
        *val = (unsigned int)lval;
    }
    return res;
}

int sc_arg2u32(const char *arg, uint32_t *val)
{
    unsigned long lval;
    int res = sc_arg2ulong(arg, &lval);
    if (res == ARGS_OK) {
        *val = (uint32_t)lval;
    }
    return res;
}

int sc_arg2s32(const char *arg, int32_t *val)
{
    unsigned long lval;
    int res = sc_arg2ulong(arg, &lval);
    if (res == ARGS_OK) {
        *val = (int32_t)lval;
    }
    return res;
}

int sc_arg2u16(const char *arg, uint16_t *val)
{
    unsigned long lval;
    int res = sc_arg2ulong(arg, &lval);
    if (res == ARGS_OK) {
        *val = (uint16_t)lval;
    }
    return res;
}

int sc_arg2u8(const char *arg, uint8_t *val)
{
    unsigned long lval;
    int res = sc_arg2ulong(arg, &lval);
    if (res == ARGS_OK) {
        *val = (uint8_t)lval;
    }
    return res;
}

int sc_arg2dev(const char *arg, unsigned maxdev)
{
    int dev = 0;
    if (sc_arg2int(arg, &dev) != ARGS_OK) {
        return ARGS_ERROR;
    }
    if (dev >= (int)maxdev) {
        return ARGS_ERROR;
    }
    return dev;
}
