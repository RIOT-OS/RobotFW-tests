*** Settings ***
Suite Setup         Run Keywords    Reset DUT and PHILIP
...                                 DUT Must Have Periph UART Application
Test Setup          Run Keywords    Reset DUT and PHILIP
...                                 DUT Must Have Periph UART Application

Resource            periph_uart.keywords.txt
Resource            api_shell.keywords.txt

Variables           test_vars.py

Force Tags          periph  uart

*** Test Cases ***
Echo
    API Call Should Succeed     Uart Init
    PHILIP.Setup Uart
    DUT Should Match String     1  ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Long Echo
    API Call Should Succeed     Uart Init
    PHILIP.Setup Uart
    DUT Should Match String     1  ${LONG_TEST_STRING}  ${LONG_TEST_STRING}
    Show PHILIP Statistics

Extended Echo
    API Call Should Succeed     Uart Init
    PHILIP.Setup Uart           mode=1
    DUT Should Match String     1  ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING_INC}
    Show PHILIP Statistics

Extended Long Echo
    API Call Should Succeed     Uart Init
    PHILIP.Setup Uart           mode=1
    DUT Should Match String     1  ${LONG_TEST_STRING}  ${LONG_TEST_STRING_INC}
    Show PHILIP Statistics

Register Access
    API Call Should Succeed     Uart Init
    PHILIP.Setup Uart           mode=2
    API Call Should Succeed     Uart Send String   ${REG_USER_READ}
    Should Be Equal             ${RESULT['data']}  ${REG_USER_READ_DATA}
    Show PHILIP Statistics

Should Not Access Invalid Register
    API Call Should Succeed     Uart Init
    PHILIP.Setup Uart           mode=2
    API Call Should Succeed     Uart Send String      ${REG_WRONG_READ}
    Should Be Equal             ${RESULT['data']}     ${REG_WRONG_READ_DATA}
    Show PHILIP Statistics

Baud Test 38400
    API Call Should Succeed     Uart Init             baud=${38400}
    PHILIP.Setup Uart           baudrate=38400
    DUT Should Match String     1  ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Baud Test 9600
    API Call Should Succeed     Uart Init             baud=${9600}
    PHILIP.Setup Uart           baudrate=9600
    DUT Should Match String     1  ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    Show PHILIP Statistics

Wrong Baud Test
    API Call Should Succeed     Uart Init             baud=${38400}
    PHILIP.Setup Uart
    DUT Should Not Match String or Timeout   1  ${SHORT_TEST_STRING}  ${SHORT_TEST_STRING}
    Show PHILIP Statistics
