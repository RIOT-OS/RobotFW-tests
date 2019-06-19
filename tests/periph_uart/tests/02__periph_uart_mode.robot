*** Settings ***
Documentation       Verify mode config functionality of the periph UART API.

Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Firmware Should Match
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset

Resource            periph_uart.keywords.txt
Resource            api_shell.keywords.txt

Variables           test_vars.py

Force Tags          periph  uart

*** Test Cases ***
Even Parity 8 Bits
    DUT UART mode should exist
    PHILIP.Setup Uart                       parity=${UART_PARITY_EVEN}
    DUT Uart Mode Change Should Succeed     data_bits=8  parity="E"  stop_bits=1
    DUT Should Match String                 ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    DUT Uart Mode Change Should Succeed     data_bits=8   parity="O"   stop_bits=1
    DUT Should Not Match String or Timeout  ${SHORT_TEST_STRING}   ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Odd Parity 8 Bits
    DUT UART mode should exist
    PHILIP.Setup Uart                       parity=${UART_PARITY_ODD}
    DUT Uart Mode Change Should Succeed     data_bits=8   parity="O"   stop_bits=1
    DUT Should Match String                 ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    DUT Uart Mode Change Should Succeed     data_bits=8   parity="E"   stop_bits=1
    DUT Should Not Match String or Timeout  ${SHORT_TEST_STRING}   ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Even Parity 7 Bits
    DUT UART mode should exist
    PHILIP.Setup Uart                       parity=${UART_PARITY_EVEN}   databits=${UART_DATA_BITS_7}
    DUT Uart Mode Change Should Succeed     data_bits=7   parity="E"   stop_bits=1
    DUT Should Match String                 ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    DUT Uart Mode Change Should Succeed     data_bits=7   parity="O"   stop_bits=1
    DUT Should Not Match String or Timeout  ${SHORT_TEST_STRING}   ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Odd Parity 7 Bits
    DUT UART mode should exist
    PHILIP.Setup Uart                       parity=${UART_PARITY_ODD}   databits=${UART_DATA_BITS_7}
    DUT Uart Mode Change Should Succeed     data_bits=7   parity="O"   stop_bits=1
    DUT Should Match String                 ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    DUT Uart Mode Change Should Succeed     data_bits=7   parity="E"   stop_bits=1
    DUT Should Not Match String or Timeout  ${SHORT_TEST_STRING}   ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Two Stop Bits
    DUT UART mode should exist
    PHILIP.Setup Uart                       parity=${UART_PARITY_ODD}
    DUT Uart Mode Change Should Succeed     data_bits=8   parity="N"   stop_bits=2
    DUT Should Match String                 ${TEST_STRING_FOR_STOP_BITS}  ${TEST_STRING_FOR_STOP_BITS}
    DUT Uart Mode Change Should Succeed     data_bits=8   parity="N"   stop_bits=1
    DUT Should Not Match String or Timeout  ${TEST_STRING_FOR_STOP_BITS}   ${TEST_STRING_FOR_STOP_BITS}
    Show PHILIP Statistics
