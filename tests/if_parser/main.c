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
 * @brief       Test application for the basic parsing tests
 *
 * @author      Kevin Weiss <kevin.weiss@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>

#include "shell.h"
#include "test_helpers.h"

#ifndef PARSER_DEV_NUM
#define PARSER_DEV_NUM 0
#endif

void print_app_metadata(int dev)
{
    print_cmd(dev,"app_metadata()");
    print_data_dict_str(dev, "app_name", RIOT_APPLICATION);
    print_data_dict_str(dev, "board", RIOT_BOARD);
    print_data_dict_str(dev, "cpu", RIOT_CPU);
#ifdef APP_SHELL_FMT
    print_data_dict_str(dev, "app_shell_fmt", APP_SHELL_FMT);
#endif
    print_data_dict_str(dev, "mcu", RIOT_MCU);
    print_data_dict_str(dev, "os_version", RIOT_VERSION);
    print_result(dev, TEST_RESULT_SUCCESS);
}

int cmd_app_metadata(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    print_app_metadata(PARSER_DEV_NUM);
    return 0;
}

int cmd_test_result_only(int argc, char **argv)
{
    (void)argv;
    if (argc > 1) {
        print_result(PARSER_DEV_NUM, TEST_RESULT_ERROR);
    }
    else {
        print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    }
    return 0;
}

int cmd_test_cmd(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    print_cmd(PARSER_DEV_NUM,"test_cmd()");
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int cmd_test_data_int(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        print_data_int(PARSER_DEV_NUM, atoi(argv[i]));
    }
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

int cmd_test_data_str(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        print_data_str(PARSER_DEV_NUM, argv[i]);
    }
    print_result(PARSER_DEV_NUM, TEST_RESULT_SUCCESS);
    return 0;
}

#ifdef JSON_SHELL_PARSER
/* Needs a forward declaration since we use shell_commands */
int cmd_help(int argc, char **argv);
#endif

static const shell_command_t shell_commands[] = {
    { "app_metadata", "Gets application metadata", cmd_app_metadata },
    { "test_result_only", "Test only result", cmd_test_result_only },
    { "test_cmd", "Test commands", cmd_test_cmd },
    { "test_data_int", "Test integers", cmd_test_data_int },
    { "test_data_str", "Test strings", cmd_test_data_str },
#ifdef JSON_SHELL_PARSER
    { "help", "Print command list", cmd_help },
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
    puts("Running app_metadata test firmware\n");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
