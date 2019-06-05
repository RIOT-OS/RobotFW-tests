*** Settings ***
Suite Setup         Run Keywords    Reset DUT and PHILIP
...                                 API Firmware Should Match
Test Setup          Run Keywords    Reset DUT and PHILIP
...                                 API Firmware Should Match

Resource            periph_uart.keywords.txt
Resource            api_shell.keywords.txt

Variables           test_vars.py

Force Tags          periph  uart

*** Test Cases ***
Even Parity 8 Bits
    DUT UART mode should exist  dev=1
    PHILIP.Setup Uart		parity=${UART_PARITY_EVEN}
    API Call Should Succeed     Uart Init
    API Call Should Succeed     Uart Mode             data_bits=8   parity="E"   stop_bits=1
    DUT Should Match String     1  ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    API Call Should Succeed     Uart Mode             data_bits=8   parity="O"   stop_bits=1
    DUT Should Not Match String or Timeout     1   ${SHORT_TEST_STRING}   ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Odd Parity 8 Bits
    DUT UART mode should exist  dev=1
    PHILIP.Setup Uart		parity=${UART_PARITY_ODD}
    API Call Should Succeed     Uart Init
    API Call Should Succeed     Uart Mode             data_bits=8   parity="O"   stop_bits=1
    DUT Should Match String     1  ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    API Call Should Succeed     Uart Mode             data_bits=8   parity="E"   stop_bits=1
    DUT Should Not Match String or Timeout     1   ${SHORT_TEST_STRING}   ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Even Parity 7 Bits
    DUT UART mode should exist  dev=1
    PHILIP.Setup Uart		parity=${UART_PARITY_EVEN}   databits=${UART_DATA_BITS_7}
    API Call Should Succeed     Uart Init
    API Call Should Succeed     Uart Mode             data_bits=7   parity="E"   stop_bits=1
    DUT Should Match String     1  ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    API Call Should Succeed     Uart Mode             data_bits=7   parity="O"   stop_bits=1
    DUT Should Not Match String or Timeout     1   ${SHORT_TEST_STRING}   ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Odd Parity 7 Bits
    DUT UART mode should exist  dev=1
    PHILIP.Setup Uart		parity=${UART_PARITY_ODD}   databits=${UART_DATA_BITS_7}
    API Call Should Succeed     Uart Init
    API Call Should Succeed     Uart Mode             data_bits=7   parity="O"   stop_bits=1
    DUT Should Match String     1  ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    API Call Should Succeed     Uart Mode             data_bits=7   parity="E"   stop_bits=1
    DUT Should Not Match String or Timeout     1   ${SHORT_TEST_STRING}   ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Two Stop Bits
    DUT UART mode should exist  dev=1
    PHILIP.Setup Uart		parity=${UART_PARITY_ODD}
    API Call Should Succeed     Uart Init
    API Call Should Succeed     Uart Mode             data_bits=8   parity="N"   stop_bits=2
    DUT Should Match String     1  ${TEST_STRING_FOR_STOP_BITS}  ${TEST_STRING_FOR_STOP_BITS}
    API Call Should Succeed     Uart Mode             data_bits=8   parity="N"   stop_bits=1
    DUT Should Not Match String or Timeout     1   ${TEST_STRING_FOR_STOP_BITS}   ${TEST_STRING_FOR_STOP_BITS}
    Show PHILIP Statistics
