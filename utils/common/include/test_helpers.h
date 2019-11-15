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

#include "shell.h"

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#ifdef JSON_SHELL_PARSER
/**
 * @name    JSON_SHELL_PARSER states
 *
 * Used to control the state of writing json formatted message.
 * @{
 */
#define JSON_STATE_READY            0
#define JSON_STATE_STARTED          0x01
#define JSON_STATE_DATA_STARTED     0x02
/** @} */

/**
 * @brief   The number of json shell parsed in case using multiple threads
 * @{
 */
#ifndef NUM_OF_JSON_SHELL_PARSER
#define NUM_OF_JSON_SHELL_PARSER    1
#endif
/** @} */

/**
 * @brief   The version of parser being used
 */
#define APP_SHELL_FMT    "JSON_SHELL_PARSER_v0.0.0"

#endif /* JSON_SHELL_PARSER */

/**
 * @name    TEST_RESULT standard states
 *
 * The proper message to send when things succeed or fail.
 * @{
 */
#define TEST_RESULT_SUCCESS "SUCCESS"
#define TEST_RESULT_ERROR   "ERROR"
/** @} */

/**
 * @brief   Prints the command that was issued to the console
 *
 * The exact output depends on the parser but it will contain information on
 * the command.
 *
 * @param[in] dev   parsing instance
 * @param[in] cmd   string of the command
 */
void print_cmd(int dev, char *cmd);

/**
 * @brief   Prints a key value where the value is a string to the console
 *
 * The exact output depends on the parser but it will contain information on
 * both the key and value.
 *
 * @param[in] dev   parsing instance
 * @param[in] key   string of the key
 * @param[in] val   string of the value
 */
void print_data_dict_str(int dev, char *key, char *val);

/**
 * @brief   Prints a int to the console
 *
 * The exact output depends on the parser but it will contain information on
 * the integer.
 *
 * @param[in] dev   parsing instance
 * @param[in] data  the integer data
 */
void print_data_int(int dev, int32_t data);

/**
 * @brief   Prints a string to the console
 *
 * The exact output depends on the parser but it will contain information on
 * the string.
 *
 * @param[in] dev   parsing instance
 * @param[in] str   the string data
 */
void print_data_str(int dev, char *str);

/**
 * @brief   Prints a result to the console
 *
 * The exact output depends on the parser but it will contain information on
 * the string.
 *
 * @note    This must be used for some parsers to indicate end of command
 *
 * @param[in] dev   parsing instance
 * @param[in] res   the TEST_RESULT string if SUCCESS or ERROR
 */
void print_result(int dev, char *res);

#endif /* TEST_HELPERS_H */
