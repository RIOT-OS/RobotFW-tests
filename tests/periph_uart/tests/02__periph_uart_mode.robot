*** Settings ***
Documentation       Verify mode config functionality of the periph UART API.

Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Firmware Should Match
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell

Resource            periph_uart.keywords.txt
Resource            api_shell.keywords.txt

Variables           test_vars.py

Force Tags          periph  uart

*** Test Cases ***
Even Parity with 8 Bits Should Succeed
    [Documentation]     Verify UART mode with 8 data bits and even parity.
    UART Mode Should Exist
    PHILIP.Setup Uart           parity=${UART_PARITY_EVEN}
    UART Mode Change Should Succeed         data_bits=8  parity="E"  stop_bits=1
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING}
    UART Mode Change Should Succeed         data_bits=8  parity="O"  stop_bits=1
    API Call Should Timeout     Uart Write  ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Odd Parity with 8 Bits Should Succeed
    [Documentation]     Verify UART mode with 8 data bits and odd parity.
    UART Mode Should Exist
    PHILIP.Setup Uart           parity=${UART_PARITY_ODD}
    UART Mode Change Should Succeed         data_bits=8  parity="O"  stop_bits=1
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING}
    UART Mode Change Should Succeed         data_bits=8  parity="E"  stop_bits=1
    API Call Should Timeout     Uart Write  ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Even Parity with 7 Bits Should Succeed
    [Documentation]     Verify UART mode with 7 data bits and even parity.
    UART Mode Should Exist
    PHILIP.Setup Uart           parity=${UART_PARITY_EVEN}   databits=${UART_DATA_BITS_7}
    UART Mode Change Should Succeed         data_bits=7  parity="E"  stop_bits=1
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING}
    UART Mode Change Should Succeed         data_bits=7  parity="O"  stop_bits=1
    API Call Should Timeout     Uart Write  ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Odd Parity with 7 Bits Should Succeed
    [Documentation]     Verify UART mode with 7 data bits and odd parity.
    UART Mode Should Exist
    PHILIP.Setup Uart           parity=${UART_PARITY_ODD}   databits=${UART_DATA_BITS_7}
    UART Mode Change Should Succeed         data_bits=7  parity="O"  stop_bits=1
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING}
    UART Mode Change Should Succeed         data_bits=7  parity="E"  stop_bits=1
    API Call Should Timeout     Uart Write  ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Write With Two Stop Bits Should Succeed
    [Documentation]     Verify UART mode with 2 stops bits.
    UART Mode Should Exist
    PHILIP.Setup Uart           parity=${UART_PARITY_ODD}
    UART Mode Change Should Succeed         data_bits=8  parity="N"  stop_bits=2
    API Call Should Succeed     Uart Write  ${TEST_STRING_FOR_STOP_BITS}
    API Result Data Should Contain  ${TEST_STRING_FOR_STOP_BITS}
    UART Mode Change Should Succeed         data_bits=8  parity="N"  stop_bits=1
    API Call Should Succeed     Uart Write  ${TEST_STRING_FOR_STOP_BITS}
    API Result Data Should Not Contain  ${TEST_STRING_FOR_STOP_BITS}
    Show PHILIP Statistics
