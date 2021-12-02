/*
 * Copyright (C) 2014 Freie Universität Berlin
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
 * @brief       Test application for the low-level I2C peripheral driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Kevin Weiss <kevin.weiss@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "periph_conf.h"
#include "periph/i2c.h"
#include "shell.h"

#include "sc_args.h"

#ifndef I2C_ACK
#define I2C_ACK         (0)
#endif

#define BUFSIZE         (128U)

/* i2c_buf is global to reduce stack memory consumtion */
static uint8_t i2c_buf[BUFSIZE];

static inline void _print_i2c_read(i2c_t dev, uint16_t *reg, uint8_t *buf, int len)
{
    printf("Success: i2c_%i read %i byte(s) ", dev, len);
    if (reg != NULL) {
        printf("from reg 0x%02x ", *reg);
    }
    printf(": [");
    for (int i = 0; i < len; i++) {
        if (i != 0) {
            printf(", ");
        }
        printf("0x%02x", buf[i]);
    }
    printf("]\n");
}

static int _print_i2c_error(int res)
{
    if (res == -EOPNOTSUPP) {
        printf("Error: EOPNOTSUPP [%d]\n", -res);
        return 1;
    }
    else if (res == -EINVAL) {
        printf("Error: EINVAL [%d]\n", -res);
        return 1;
    }
    else if (res == -EAGAIN) {
        printf("Error: EAGAIN [%d]\n", -res);
        return 1;
    }
    else if (res == -ENXIO) {
        printf("Error: ENXIO [%d]\n", -res);
        return 1;
    }
    else if (res == -EIO) {
        printf("Error: EIO [%d]\n", -res);
        return 1;
    }
    else if (res == -ETIMEDOUT) {
        printf("Error: ETIMEDOUT [%d]\n", -res);
        return 1;
    }
    else if (res == I2C_ACK) {
        printf("Success: I2C_ACK [%d]\n", res);
        return 0;
    }
    printf("Error: Unknown error [%d]\n", res);
    return 1;
}

int cmd_i2c_acquire(int argc, char **argv)
{

    int res = sc_args_check(argc, argv, 1, 1, "DEV");
    if (res != ARGS_OK) {
        return 1;
    }

    int dev = sc_arg2dev(argv[1], I2C_NUMOF);
    if (dev < 0) {
        return -ENODEV;
    }
    printf("Command: i2c_acquire(%i)\n", dev);
    i2c_acquire(dev);
    printf("Success: i2c_%i acquired\n", dev);
    return 0;
}

int cmd_i2c_release(int argc, char **argv)
{
    int res = sc_args_check(argc, argv, 1, 1, "DEV");
    if (res != ARGS_OK) {
        return 1;
    }

    int dev = sc_arg2dev(argv[1], I2C_NUMOF);
    if (dev < 0) {
        return -ENODEV;
    }

    printf("Command: i2c_release(%i)\n", dev);
    i2c_release(dev);

    printf("Success: i2c_%i released\n", dev);
    return 0;
}

int cmd_i2c_read_reg(int argc, char **argv)
{
    int res = sc_args_check(argc, argv, 4, 4, "DEV ADDR REG FLAG");
    if (res != ARGS_OK) {
        return 1;
    }

    int dev = sc_arg2dev(argv[1], I2C_NUMOF);
    if (dev < 0) {
        return -ENODEV;
    }

    uint16_t addr = 0;
    uint16_t reg = 0;
    uint8_t flags = 0;

    if (sc_arg2u16(argv[2], &addr) != ARGS_OK
        || sc_arg2u16(argv[3], &reg) != ARGS_OK
        || sc_arg2u8(argv[4], &flags) != ARGS_OK) {
        return 1;
    }

    printf("Command: i2c_read_reg(%i, 0x%02x, 0x%02x, 0x%02x)\n",
           dev, addr, reg, flags);
    uint8_t data;
    res = i2c_read_reg(dev, addr, reg, &data, flags);

    if (res == I2C_ACK) {
        _print_i2c_read(dev, &reg, &data, 1);
        return 0;
    }
    return _print_i2c_error(res);
}

int cmd_i2c_read_regs(int argc, char **argv)
{
    int res = sc_args_check(argc, argv, 5, 5, "DEV ADDR REG LEN FLAG");
    if (res != ARGS_OK) {
        return 1;
    }

    int dev = sc_arg2dev(argv[1], I2C_NUMOF);
    if (dev < 0) {
        return -ENODEV;
    }

    uint16_t addr = 0;
    uint16_t reg = 0;
    int len = 0;
    uint8_t flags = 0;

    if (sc_arg2u16(argv[2], &addr) != ARGS_OK
        || sc_arg2u16(argv[3], &reg) != ARGS_OK
        || sc_arg2int(argv[4], &len) != ARGS_OK
        || sc_arg2u8(argv[5], &flags) != ARGS_OK) {
        return 1;
    }

    if (len < 1 || len > (int)BUFSIZE) {
        puts("Error: invalid LENGTH parameter given");
        return 1;
    }
    else {
        printf("Command: i2c_read_regs(%i, 0x%02x, 0x%02x, %i, 0x%02x)\n",
               dev, addr, reg, len, flags);
        res = i2c_read_regs(dev, addr, reg, i2c_buf, len, flags);
    }

    if (res == I2C_ACK) {
        _print_i2c_read(dev, &reg, i2c_buf, len);
        return 0;
    }
    return _print_i2c_error(res);
}

int cmd_i2c_read_byte(int argc, char **argv)
{
    int res = sc_args_check(argc, argv, 3, 3, "DEV ADDR FLAG");
    if (res != ARGS_OK) {
        return 1;
    }

    int dev = sc_arg2dev(argv[1], I2C_NUMOF);
    if (dev < 0) {
        return -ENODEV;
    }

    uint16_t addr = 0;
    uint8_t flags = 0;

    if (sc_arg2u16(argv[2], &addr) != ARGS_OK
        || sc_arg2u8(argv[3], &flags) != ARGS_OK) {
        return 1;
    }

    printf("Command: i2c_read_byte(%i, 0x%02x, 0x%02x)\n", dev, addr, flags);
    uint8_t data;
    res = i2c_read_byte(dev, addr, &data, flags);

    if (res == I2C_ACK) {
        _print_i2c_read(dev, NULL, &data, 1);
        return 0;
    }
    return _print_i2c_error(res);
}

int cmd_i2c_read_bytes(int argc, char **argv)
{
    int res = sc_args_check(argc, argv, 4, 4, "DEV ADDR LENGTH FLAG");
    if (res != ARGS_OK) {
        return 1;
    }

    int dev = sc_arg2dev(argv[1], I2C_NUMOF);
    if (dev < 0) {
        return -ENODEV;
    }

    uint16_t addr = 0;
    int len = 0;
    uint8_t flags = 0;

    if (sc_arg2u16(argv[2], &addr) != ARGS_OK
        || sc_arg2int(argv[3], &len) != ARGS_OK
        || sc_arg2u8(argv[4], &flags) != ARGS_OK) {
        return 1;
    }

    if (len < 1 || len > (int)BUFSIZE) {
        puts("Error: invalid LENGTH parameter given");
        return 1;
    }
    else {
        printf("Command: i2c_read_bytes(%i, 0x%02x, %i, 0x%02x)\n", dev,
         addr, len, flags);
        res = i2c_read_bytes(dev, addr, i2c_buf, len, flags);
    }

    if (res == I2C_ACK) {
        _print_i2c_read(dev, NULL, i2c_buf, len);
        return 0;
    }
    return _print_i2c_error(res);
}

int cmd_i2c_write_byte(int argc, char **argv)
{
    int res = sc_args_check(argc, argv, 4, 4, "DEV ADDR BYTE FLAG");
    if (res != ARGS_OK) {
        return 1;
    }

    int dev = sc_arg2dev(argv[1], I2C_NUMOF);
    if (dev < 0) {
        return -ENODEV;
    }

    uint16_t addr = 0;
    uint8_t data = 0;
    uint8_t flags = 0;

    if (sc_arg2u16(argv[2], &addr) != ARGS_OK
        || sc_arg2u8(argv[3], &data) != ARGS_OK
        || sc_arg2u8(argv[4], &flags) != ARGS_OK) {
        return 1;
    }

    printf("Command: i2c_write_byte(%i, 0x%02x, 0x%02x, [0x%02x",
           dev, addr, flags, data);
    puts("])");
    res = i2c_write_byte(dev, addr, data, flags);

    if (res == I2C_ACK) {
        printf("Success: i2c_%i wrote 1 byte to the bus\n", dev);
        return 0;
    }
    return _print_i2c_error(res);
}

int cmd_i2c_write_bytes(int argc, char **argv)
{
    int res = sc_args_check(argc, argv, 4, 3 + BUFSIZE,
                            "DEV ADDR FLAG BYTE0 [BYTE1 [BYTE_n [...]]]");
    if (res != ARGS_OK) {
        return 1;
    }

    int dev = sc_arg2dev(argv[1], I2C_NUMOF);
    if (dev < 0) {
        return -ENODEV;
    }

    uint16_t addr = 0;;
    uint8_t flags = 0;

    if (sc_arg2u16(argv[2], &addr) != ARGS_OK
        || sc_arg2u8(argv[3], &flags) != ARGS_OK) {
        return 1;
    }
    int len = argc - 4;
    for (int i = 0; i < len; i++) {
        if (sc_arg2u8(argv[i + 4], &i2c_buf[i]) != ARGS_OK) {
            return 1;
        }
    }

    printf("Command: i2c_write_bytes(%i, 0x%02x, 0x%02x, [", dev, addr, flags);
    for (int i = 0; i < (argc - 4); i++) {
        if (i != 0) {
            printf(", ");
        }
        printf("0x%02x", i2c_buf[i]);
    }
    puts("])");
    res = i2c_write_bytes(dev, addr, i2c_buf, len, flags);

    if (res == I2C_ACK) {
        printf("Success: i2c_%i wrote %i bytes\n", dev, len);
        return 0;
    }
    return _print_i2c_error(res);
}

int cmd_i2c_write_reg(int argc, char **argv)
{
    int res = sc_args_check(argc, argv, 5, 5, "DEV ADDR REG BYTE FLAG");
    if (res != ARGS_OK) {
        return 1;
    }

    int dev = sc_arg2dev(argv[1], I2C_NUMOF);
    if (dev < 0) {
        return -ENODEV;
    }

    uint16_t addr = 0;
    uint16_t reg = 0;
    uint8_t data = 0;
    uint8_t flags = 0;

    if (sc_arg2u16(argv[2], &addr) != ARGS_OK
        || sc_arg2u16(argv[3], &reg) != ARGS_OK
        || sc_arg2u8(argv[4], &data) != ARGS_OK
        || sc_arg2u8(argv[5], &flags) != ARGS_OK) {
        return 1;
    }

    printf("Command: i2c_write_reg(%i, 0x%02x, 0x%02x, 0x%02x, [0x%02x",
           dev, addr, reg, flags, data);
    puts("])");
    res = i2c_write_reg(dev, addr, reg, data, flags);

    if (res == I2C_ACK) {
        printf("Success: i2c_%i wrote 1 byte\n", dev);
        return 0;
    }
    return _print_i2c_error(res);
}

int cmd_i2c_write_regs(int argc, char **argv)
{
    int res = sc_args_check(argc, argv, 5, 4 + BUFSIZE, "DEV ADDR REG FLAG BYTE0 [BYTE1 ...]");
    if (res != ARGS_OK) {
        return 1;
    }

    int dev = sc_arg2dev(argv[1], I2C_NUMOF);
    if (dev < 0) {
        return -ENODEV;
    }

    uint16_t addr = 0;
    uint16_t reg = 0;
    uint8_t flags = 0;

    if (sc_arg2u16(argv[2], &addr) != ARGS_OK
        || sc_arg2u16(argv[3], &reg) != ARGS_OK
        || sc_arg2u8(argv[4], &flags) != ARGS_OK) {
        return 1;
    }
    int len = argc - 5;
    for (int i = 0; i < len; i++) {
        if (sc_arg2u8(argv[i + 5], &i2c_buf[i]) != ARGS_OK) {
            return 1;
        }
    }

    printf("Command: i2c_write_regs(%i, 0x%02x, 0x%02x, 0x%02x, [",
           dev, addr, reg, flags);
    for (int i = 0; i < len; i++) {
        if (i != 0) {
            printf(", ");
        }
        printf("0x%02x", i2c_buf[i]);
    }
    puts("])");
    res = i2c_write_regs(dev, addr, reg, i2c_buf, len, flags);

    if (res == I2C_ACK) {
        printf("Success: i2c_%i wrote %i bytes to reg 0x%02x\n",
            dev, len, reg);
        return 0;
    }
    return _print_i2c_error(res);
}

int cmd_i2c_get_devs(int argc, char **argv)
{
    (void)argv;
    (void)argc;

    printf("Command: return I2C_NUMOF\n");
    printf("Success: Amount of i2c devices: [%d]\n", I2C_NUMOF);
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
    { "i2c_acquire", "Get access to the I2C bus", cmd_i2c_acquire },
    { "i2c_release", "Release to the I2C bus", cmd_i2c_release },
    { "i2c_read_reg", "Read byte from register", cmd_i2c_read_reg },
    { "i2c_read_regs", "Read bytes from registers", cmd_i2c_read_regs },
    { "i2c_read_byte", "Read byte from the I2C device", cmd_i2c_read_byte },
    { "i2c_read_bytes", "Read bytes from the I2C device", cmd_i2c_read_bytes },
    { "i2c_write_byte", "Write byte to the I2C device", cmd_i2c_write_byte },
    { "i2c_write_bytes", "Write bytes to the I2C device", cmd_i2c_write_bytes },
    { "i2c_write_reg", "Write byte to register", cmd_i2c_write_reg },
    { "i2c_write_regs", "Write bytes to registers", cmd_i2c_write_regs },
    { "i2c_get_devs", "Gets amount of supported i2c devices", cmd_i2c_get_devs },
    { "get_metadata", "Get the metadata of the test firmware", cmd_get_metadata },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("Start: Test for the low-level I2C driver");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
