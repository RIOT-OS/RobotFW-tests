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
    PHILIP Setup UART
    UART Init and Flush Should Succeed
    Uart Write Should Succeed       ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING}
    PHILIP Log Stats

Long Echo Should Succeed
    [Documentation]     Write long string to UART and verify echo reply.
    PHILIP Setup UART
    UART Init and Flush Should Succeed
    Uart Write Should Succeed       ${LONG_TEST_STRING}
    API Result Data Should Contain  ${LONG_TEST_STRING}
    PHILIP Log Stats

Extended Short Echo Should Succeed
    [Documentation]     Verify echo of short string to UART.
    PHILIP Setup UART  mode=1
    UART Init and Flush Should Succeed
    Uart Write Should Succeed       ${SHORT_TEST_STRING}
    API Result Data Should Contain  ${SHORT_TEST_STRING_INC}
    PHILIP Log Stats

Extended Long Echo Should Succeed
    [Documentation]     Verify echo of long string to UART.
    PHILIP Setup UART  mode=1
    UART Init and Flush Should Succeed
    Uart Write Should Succeed       ${LONG_TEST_STRING}
    API Result Data Should Contain  ${LONG_TEST_STRING_INC}
    PHILIP Log Stats

Register Access Should Succeed
    [Documentation]     Verify access of user register via UART.
    PHILIP Setup UART  mode=2
    UART Init and Flush Should Succeed
    Uart Write Should Succeed   ${REG_USER_READ}
    Should Be Equal             ${RESULT['data']}  ${REG_USER_READ_DATA}
    PHILIP Log Stats

Invalid Register Access Should Fail
    [Documentation]     Verfiy access of invalid register via UART fails.
    PHILIP Setup UART           mode=2
    UART Init and Flush Should Succeed
    Uart Write Should Succeed   ${REG_WRONG_READ}
    Should Be Equal             ${RESULT['data']}  ${REG_WRONG_READ_DATA}
    PHILIP Log Stats

Baudrate 9600 Should Succeed
    [Documentation]     Verify UART write with baudrate 9600.
    PHILIP Setup UART                       baudrate=9600
    UART Init and Flush Should Succeed      baud=${9600}
    Uart Write Should Succeed               ${SHORT_TEST_STRING}
    API Result Data Should Contain          ${SHORT_TEST_STRING}
    PHILIP Log Stats

Baudrate 38400 Should Succeed
    [Documentation]     Verify UART write with baudrate 38400.
    PHILIP Setup UART                       baudrate=38400
    UART Init and Flush Should Succeed      baud=${38400}
    Uart Write Should Succeed               ${SHORT_TEST_STRING}
    API Result Data Should Contain          ${SHORT_TEST_STRING}
    PHILIP Log Stats

Baudrate Mismatch Should Fail
    [Documentation]     Verify UART write fails when baudrates do not match.
    PHILIP Setup UART                       baudrate=9600
    UART Init and Flush Should Succeed      baud=${38400}
    Uart Write Should Timeout               ${SHORT_TEST_STRING}
    PHILIP Log Stats
