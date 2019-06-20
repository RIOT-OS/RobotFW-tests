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
Short Echo Should Succeed
    [Documentation]     Write short string to UART and verify echo reply.
    PHILIP.Setup Uart
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING}
    Log                 PHILIP Stats

Long Echo Should Succeed
    [Documentation]     Write long string to UART and verify echo reply.
    PHILIP.Setup Uart
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write  ${LONG_TEST_STRING}
    API Result Data Should Contain  ${LONG_TEST_STRING}
    Log                 PHILIP Stats

Extended Short Echo Should Succeed
    [Documentation]     Verify echo of short string to UART.
    PHILIP.Setup Uart           mode=1
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING_INC}
    Log                 PHILIP Stats

Extended Long Echo Should Succeed
    [Documentation]     Verify echo of long string to UART.
    PHILIP.Setup Uart           mode=1
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write  ${LONG_TEST_STRING}
    API Result Data Should Contain  ${LONG_TEST_STRING_INC}
    Log                 PHILIP Stats

Register Access Should Succeed
    [Documentation]     Verify access of user register via UART.
    PHILIP.Setup Uart           mode=2
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write          ${REG_USER_READ}
    Should Be Equal             ${RESULT['data']}   ${REG_USER_READ_DATA}
    Log                 PHILIP Stats

Invalid Register Access Should Fail
    [Documentation]     Verfiy access of invalid register via UART fails.
    PHILIP.Setup Uart           mode=2
    UART Init and Flush Should Succeed
    API Call Should Succeed     Uart Write          ${REG_WRONG_READ}
    Should Be Equal             ${RESULT['data']}   ${REG_WRONG_READ_DATA}
    Log                 PHILIP Stats

Baudrate 9600 Should Succeed
    [Documentation]     Verify UART write with baudrate 9600.
    PHILIP.Setup Uart           baudrate=9600
    UART Init and Flush Should Succeed      baud=${9600}
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING}
    Log                 PHILIP Stats

Baudrate 38400 Should Succeed
    [Documentation]     Verify UART write with baudrate 9600.
    PHILIP.Setup Uart           baudrate=38400
    UART Init and Flush Should Succeed      baud=${38400}
    API Call Should Succeed     Uart Write  ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING}
    Log                 PHILIP Stats

Baudrate Mismatch Should Fail
    [Documentation]     Verify UART write fails when baudrates do not match.
    PHILIP.Setup Uart           baudrate=9600
    UART Init and Flush Should Succeed      baud=${38400}
    API Call Should Timeout     Uart Write  ${SHORT_TEST_STRING}
    Log                 PHILIP Stats
