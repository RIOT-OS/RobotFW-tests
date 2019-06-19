*** Settings ***
Documentation       Verify basic functionality of the periph UART API.

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
Echo
    PHILIP.Setup Uart
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Long Echo
    PHILIP.Setup Uart
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write  ${LONG_TEST_STRING}
    API Result Data Should Contain  ${LONG_TEST_STRING}
    Show PHILIP Statistics

Extended Echo
    PHILIP.Setup Uart           mode=1
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING_INC}
    Show PHILIP Statistics

Extended Long Echo
    PHILIP.Setup Uart           mode=1
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write  ${LONG_TEST_STRING}
    API Result Data Should Contain  ${LONG_TEST_STRING_INC}
    Show PHILIP Statistics

Register Access
    PHILIP.Setup Uart           mode=2
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write          ${REG_USER_READ}
    Should Be Equal             ${RESULT['data']}   ${REG_USER_READ_DATA}
    Show PHILIP Statistics

Should Not Access Invalid Register
    PHILIP.Setup Uart           mode=2
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write          ${REG_WRONG_READ}
    Should Be Equal             ${RESULT['data']}   ${REG_WRONG_READ_DATA}
    Show PHILIP Statistics

Baud Test 38400
    PHILIP.Setup Uart           baudrate=38400
    UART Init and Flush Should Succeed      baud=${38400}
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Baud Test 9600
    PHILIP.Setup Uart           baudrate=9600
    UART Init and Flush Should Succeed      baud=${9600}
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Wrong Baud Test
    PHILIP.Setup Uart           baudrate=9600
    UART Init and Flush Should Succeed      baud=${38400}
    API Call Should Timeout     Uart Write  ${SHORT_TEST_STRING}
    Show PHILIP Statistics
