/*
 * Copyright (C) 2014 Freie Universität Berlin
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
 * @brief       Test application for the low-level SPI peripheral driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Kevin Weiss <kevin.weiss@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include "periph_conf.h"
#include "periph/spi.h"
#include "shell.h"
#include "test_helpers.h"
#include "sc_args.h"

#ifndef PARSER_DEV_NUM
#define PARSER_DEV_NUM 0
#endif

#ifndef FULL_ASSERT
#define FULL_ASSERT 0
#endif

#if FULL_ASSERT
#define CHECK_ASSERT(x, usage)     assert(x)
#else
#define CHECK_ASSERT(x, usage)     do { \
                                if (!(x)) { \
                                    puts(usage); \
                                    print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR); \
                                    return -1; \
                                } \
                            } while(0)
#endif

static struct {
    spi_t dev;
    spi_mode_t mode;
    spi_clk_t clk;
    spi_cs_t cs;
} spiconf;

char printbuf[SHELL_DEFAULT_BUFSIZE] = {0};
uint8_t in_buf[64];
uint8_t out_buf[64];

int cmd_spi_init(int argc, char **argv)
{
    const char* USAGE = "spi_init DEV";
    CHECK_ASSERT(argc == 2, USAGE);
    int dev = sc_arg2dev(argv[1], SPI_NUMOF);
    CHECK_ASSERT(dev != ARGS_ERROR, USAGE);

    sprintf(printbuf, "spi_init(dev=%i)", dev);
    print_cmd(PARSER_DEV_NUM, printbuf);
    spi_init(dev);
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int cmd_spi_init_pins(int argc, char **argv)
{
    const char* USAGE = "spi_init_pins DEV";
    CHECK_ASSERT(argc == 2, USAGE);
    int dev = sc_arg2dev(argv[1], SPI_NUMOF);
    CHECK_ASSERT(dev != ARGS_ERROR, USAGE);

    sprintf(printbuf, "spi_init_pins(dev=%i)", dev);

    print_cmd(PARSER_DEV_NUM, printbuf);
    spi_init_pins(dev);
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int cmd_spi_init_cs(int argc, char **argv)
{
    const char* USAGE = "spi_init_cs DEV CS_PORT CS_PIN";
    CHECK_ASSERT(argc == 4, USAGE);
    int dev = sc_arg2dev(argv[1], SPI_NUMOF);
    int32_t port, pin = 0;
    CHECK_ASSERT(dev != ARGS_ERROR &&
        (sc_arg2s32(argv[2], &port) == ARGS_OK) &&
        (sc_arg2s32(argv[3], &pin) == ARGS_OK), USAGE);

    sprintf(printbuf, "spi_init_cs(dev=%i, port=%"PRIi32", pin=%"PRIi32")",
            dev, port, pin);

    print_cmd(PARSER_DEV_NUM, printbuf);
    spi_init_cs(dev, GPIO_PIN(port, pin));
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int cmd_spi_init_with_gpio_mode(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    /* Not implemented */
    print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
    return 0;
}

int cmd_spi_acquire(int argc, char **argv)
{
    const char* USAGE = "spi_acquire DEV MODE 100k|400k|1M|5M|10M CS_PORT CS_PIN";
    CHECK_ASSERT(argc == 6, USAGE);
    int dev = sc_arg2dev(argv[1], SPI_NUMOF);
    int mode = 0;
    int32_t port, pin = 0;
    CHECK_ASSERT(dev != ARGS_ERROR &&
        (sc_arg2int(argv[2], &mode) == ARGS_OK) &&
        (sc_arg2s32(argv[4], &port) == ARGS_OK) &&
        (sc_arg2s32(argv[5], &pin) == ARGS_OK), USAGE);

    spiconf.dev = SPI_DEV(dev);

    /* parse the SPI mode */
    mode = atoi(argv[2]);
    switch (mode) {
        case 0:
            spiconf.mode = SPI_MODE_0;
            break;
        case 1:
            spiconf.mode = SPI_MODE_1;
            break;
        case 2:
            spiconf.mode = SPI_MODE_2;
            break;
        case 3:
            spiconf.mode = SPI_MODE_3;
            break;
        default:
            CHECK_ASSERT(0, USAGE);
    }

    if (strcmp(argv[3], "100k") == 0) {
        spiconf.clk = SPI_CLK_100KHZ;
    }
    else if (strcmp(argv[3], "400k") == 0) {
        spiconf.clk = SPI_CLK_400KHZ;
    }
    else if (strcmp(argv[3], "1M") == 0) {
        spiconf.clk = SPI_CLK_1MHZ;
    }
    else if (strcmp(argv[3], "5M") == 0) {
        spiconf.clk = SPI_CLK_5MHZ;
    }
    else if (strcmp(argv[3], "10M") == 0) {
        spiconf.clk = SPI_CLK_10MHZ;
    }
    else {
        CHECK_ASSERT(0, USAGE);
    }

    if (port == -1) {                    /* hardware chip select line */
        spiconf.cs = SPI_HWCS(pin);
    }
    else {
        spiconf.cs = (spi_cs_t)GPIO_PIN(port, pin);
    }
    sprintf(printbuf,
            "spi_acquire(bus=%i, port=%"PRIi32", pin=%"PRIi32", mode=%i, clk=%s)",
            dev, port, pin, mode, argv[3]);
    print_cmd(PARSER_DEV_NUM, printbuf);
    spi_acquire(spiconf.dev, spiconf.cs, spiconf.mode, spiconf.clk);
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int cmd_spi_release(int argc, char **argv)
{
    const char* USAGE = "spi_release DEV";
    CHECK_ASSERT(argc == 2, USAGE);
    int dev = sc_arg2dev(argv[1], SPI_NUMOF);
    CHECK_ASSERT(dev != ARGS_ERROR, USAGE);
    sprintf(printbuf, "spi_release(dev=%i)", dev);
    print_cmd(PARSER_DEV_NUM, printbuf);
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    spi_release(dev);
    return 0;
}

int cmd_spi_transfer_byte(int argc, char **argv)
{
    const char* USAGE = "spi_transfer_byte DEV CS_PORT CS_PIN CONT OUT";
    CHECK_ASSERT(argc == 6, USAGE);
    int dev = sc_arg2dev(argv[1], SPI_NUMOF);
    int32_t port, pin;
    int cont;
    uint8_t out;
    CHECK_ASSERT(dev != ARGS_ERROR &&
        (sc_arg2int(argv[4], &cont) == ARGS_OK) &&
        (sc_arg2s32(argv[2], &port) == ARGS_OK) &&
        (sc_arg2s32(argv[3], &pin) == ARGS_OK) &&
        (sc_arg2u8(argv[5], &out) == ARGS_OK), USAGE);

    sprintf(printbuf,
           "spi_transfer_byte(dev=%i, port=%"PRIi32", pin=%"PRIi32", cont=%i, out=%u)",
            dev, port, pin, cont, out);

    print_cmd(PARSER_DEV_NUM, printbuf);
    print_data_int(PARSER_DEV_NUM,
                   spi_transfer_byte(dev,
                                     GPIO_PIN(port, pin),
                                     cont,
                                     out));
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int cmd_spi_transfer_bytes(int argc, char **argv)
{
    const char* USAGE = "spi_transfer_bytes DEV CS_PORT CS_PIN CONT IN_LEN [OUT0...OUTn]";
    CHECK_ASSERT(argc >= 6, USAGE);
    int dev = sc_arg2dev(argv[1], SPI_NUMOF);
    int32_t port, pin;
    int cont;
    unsigned int in_len;
    uint8_t *out = NULL;
    uint8_t *in = NULL;
    unsigned int len;
    CHECK_ASSERT(dev != ARGS_ERROR &&
        (sc_arg2int(argv[4], &cont) == ARGS_OK) &&
        (sc_arg2uint(argv[5], &in_len) == ARGS_OK) &&
        (sc_arg2s32(argv[2], &port) == ARGS_OK) &&
        (sc_arg2s32(argv[3], &pin) == ARGS_OK), USAGE);

    int offset = sprintf(printbuf,
            "spi_transfer_bytes(dev=%i, port=%"PRIi32", pin=%"PRIi32", cont=%i, out=",
            dev, port, pin, cont);
    if (in_len) {
        in = in_buf;
    }
    if (argc == 6) {
        offset += sprintf(printbuf + offset, "NULL ");
        len = in_len;
    }
    else {
        out = out_buf;
        len = argc - 6;
        for (unsigned int i = 0; i < len; i++) {
            CHECK_ASSERT(sc_arg2u8(argv[6 + i], &out[i]) == ARGS_OK, "Could not parse OUT bytes");
            offset += sprintf(printbuf + offset, "%u ", out[i]);
        }
    }
    sprintf(printbuf + offset, "in=%s len=%u)", in_len ? "data" : "NULL", len);

    print_cmd(PARSER_DEV_NUM, printbuf);
    spi_transfer_bytes(dev, GPIO_PIN(port, pin), cont, out, in, len);

    if (in != NULL) {
        for (unsigned int i = 0; i < len; i++) {
            print_data_int(PARSER_DEV_NUM, in[i]);
        }
    }

    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int cmd_spi_transfer_reg(int argc, char **argv)
{
    const char* USAGE = "spi_transfer_reg DEV CS_PORT CS_PIN REG OUT";
    CHECK_ASSERT(argc == 6, USAGE);
    int dev = sc_arg2dev(argv[1], SPI_NUMOF);
    int32_t port, pin;
    unsigned int reg;
    uint8_t out;
    CHECK_ASSERT(dev != ARGS_ERROR &&
        (sc_arg2uint(argv[4], &reg) == ARGS_OK) &&
        (sc_arg2s32(argv[2], &port) == ARGS_OK) &&
        (sc_arg2s32(argv[3], &pin) == ARGS_OK) &&
        (sc_arg2u8(argv[5], &out) == ARGS_OK), USAGE);

    sprintf(printbuf,
           "spi_transfer_reg(dev=%i, port=%"PRIi32", pin=%"PRIi32", reg=%u, out=%u)",
            dev, port, pin, reg, out);

    print_cmd(PARSER_DEV_NUM, printbuf);
    print_data_int(PARSER_DEV_NUM,
                   spi_transfer_reg(dev, GPIO_PIN(port, pin), reg, out));
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int cmd_spi_transfer_regs(int argc, char **argv)
{
    const char* USAGE = "spi_transfer_regs DEV CS_PORT CS_PIN REG IN_LEN [OUT0...OUTn]";
    CHECK_ASSERT(argc >= 6, USAGE);
    int dev = sc_arg2dev(argv[1], SPI_NUMOF);
    int32_t port, pin;
    unsigned int reg;
    unsigned int in_len;
    uint8_t *out = NULL;
    uint8_t *in = NULL;
    unsigned int len;
    CHECK_ASSERT(dev != ARGS_ERROR &&
        (sc_arg2uint(argv[4], &reg) == ARGS_OK) &&
        (sc_arg2uint(argv[5], &in_len) == ARGS_OK) &&
        (sc_arg2s32(argv[2], &port) == ARGS_OK) &&
        (sc_arg2s32(argv[3], &pin) == ARGS_OK), USAGE);

    int offset = sprintf(printbuf,
            "spi_transfer_regs(dev=%i, port=%"PRIi32", pin=%"PRIi32", reg=%u, out=",
            dev, port, pin, reg);
    if (in_len) {
        in = in_buf;
    }
    if (argc == 6) {
        offset += sprintf(printbuf + offset, "NULL ");
        len = in_len;
    }
    else {
        len = argc - 6;
        out = out_buf;
        for (unsigned int i = 0; i < len; i++) {
            CHECK_ASSERT(sc_arg2u8(argv[6 + i], &out[i]) == ARGS_OK, "Could not parse OUT bytes");
            offset += sprintf(printbuf + offset, "%u ", out[i]);
        }
    }
    sprintf(printbuf + offset, "in=%s len=%u", in_len ? "data" : "NULL", len);

    print_cmd(PARSER_DEV_NUM, printbuf);
    spi_transfer_regs(dev, GPIO_PIN(port, pin), reg, out, in, len);

    if (in != NULL) {
        for (unsigned int i = 0; i < len; i++) {
            print_data_int(PARSER_DEV_NUM, in_buf[i]);
        }
    }

    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int cmd_get_metadata(int argc, char **argv)
{
    (void)argv;
    (void)argc;
    print_data_str(PARSER_DEV_NUM, RIOT_BOARD);
    print_data_str(PARSER_DEV_NUM, RIOT_APPLICATION);
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);

    return 0;
}


#ifdef JSON_SHELL_PARSER
/* Needs a forward declaration since we use shell_commands */
int cmd_help(int argc, char **argv);
#endif

static const shell_command_t shell_commands[] = {
    { "spi_init", "Basic initialization of the given SPI bus", cmd_spi_init },
    { "spi_init_pins", "Initialize the used SPI bus pins, i.e. MISO, MOSI, and CLK", cmd_spi_init_pins },
    { "spi_init_cs", "Initialize the given chip select pin", cmd_spi_init_cs },
    { "spi_init_with_gpio_mode", "Initialize MOSI/MISO/SCLK pins with adapted GPIO modes", cmd_spi_init_with_gpio_mode },
    { "spi_acquire", "Start a new SPI transaction", cmd_spi_acquire },
    { "spi_release", "Finish an ongoing SPI transaction by releasing the given SPI bus", cmd_spi_release },
    { "spi_transfer_byte", "Transfer one byte on the given SPI bus", cmd_spi_transfer_byte },
    { "spi_transfer_bytes", "Transfer a number bytes using the given SPI bus", cmd_spi_transfer_bytes },
    { "spi_transfer_reg", "Transfer one byte to/from a given register address", cmd_spi_transfer_reg },
    { "spi_transfer_regs", "Transfer a number bytes using the given SPI bus", cmd_spi_transfer_regs },
    { "get_metadata", "Get the metadata of the test firmware", cmd_get_metadata },
#ifdef JSON_SHELL_PARSER
    { "help", "Override help for parsable help options", cmd_help },
#endif
    { NULL, NULL, NULL }
};

#ifdef JSON_SHELL_PARSER
int cmd_help(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    for (unsigned int i = 0; shell_commands[i].name != NULL; i++) {
        print_data_str(PARSER_DEV_NUM, (char*)shell_commands[i].name);
    }
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}
#endif

int main(void)
{
    puts("Start: tests/periph_spi");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
