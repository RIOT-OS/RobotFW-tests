*** Settings ***
Documentation       Verify basic functionality of the periph I2C API.

Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Firmware Should Match
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell

Resource            periph_i2c.keywords.txt
Resource            api_shell.keywords.txt

Force Tags          periph  i2c

*** Test Cases ***
Acquire and Release Should Succeed
    [Documentation]             Verify I2C acquire and release API calls.
    API Call Should Succeed     I2C Acquire
    API Call Should Succeed     I2C Release

Acquire after Release Should Succeed
    [Documentation]             Verify acquiring an I2C bus after release.
    API Call Should Succeed     I2C Acquire
    API Call Should Succeed     I2C Release
    API Call Should Succeed     I2C Acquire

CUSTOM TEST
    [Documentation]             Verify acquiring an I2C bus after release.
    API Call Should Succeed     I2C Acquire
    API Call Should Succeed     I2C Read Bytes Loop  leng=100
    Record Property             "Test"    ${RESULT}
    API Call Should Succeed     I2C Read Bytes Loop  leng=100  loop=10
    Record Property             "Test"    ${RESULT}
    API Call Should Succeed     I2C Read Bytes Loop  leng=10  loop=100
    Record Property             "Test"    ${RESULT}
    API Call Should Succeed     I2C Release

Double Acquire Should Timeout
    [Documentation]             Verify that acquiring a locked I2C bus blocks.
    API Call Should Succeed     I2C Acquire
    API Call Should Timeout     I2C Acquire

Read Register After NACK Should Succeed
    [Documentation]             Verify recovery of I2C bus after NACK.
    API Call Should Succeed     I2C Acquire
    API Call Should Error       I2C Read Reg  addr=42
    API Call Should Succeed     I2C Read Reg
    API Call Should Succeed     I2C Release

Read Byte After Multiple NACKs Should Succeed
    [Documentation]             Verify recovery of read bytes NACK.
    API Call Should Succeed     I2C Acquire
    API Call Should Error       I2C Read Byte  addr=42
    API Call Should Error       I2C Read Byte  addr=43
    API Call Should Succeed     I2C Read Byte
    API Call Should Succeed     I2C Release

Pins High After Release Should Succeed
    [Documentation]             Verify pins are high after release.
    API Call Should Succeed     I2C Acquire
    API Call Should Succeed     I2C Release
    API Call Should Succeed     PHiLIP.read reg  i2c.dut_sda.level
    Should Be Equal             ${RESULT['data']}  ${1}
    API Call Should Succeed     PHiLIP.read reg  i2c.dut_scl.level
    Should Be Equal             ${RESULT['data']}  ${1}
