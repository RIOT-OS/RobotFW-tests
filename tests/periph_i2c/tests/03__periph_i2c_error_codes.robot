*** Settings ***
Documentation       Tests to verify correct error codes are given.

Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Firmware Should Match
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell
...                                 I2C Acquire
Test Teardown       I2C Release

Resource            periph_i2c.keywords.txt
Resource            api_shell.keywords.txt
Resource            riot_base.keywords.txt

Force Tags          periph  i2c

*** Test Cases ***
Invalid Address Should Fail With Address NACK
    [Documentation]                         Verify address NACK (ENXIO) occurs
    API Call Should Error                   I2C Read Reg  addr=42
    API Result Message Should Contain       ENXIO

Invalid Data Should Fail With Data NACK
    [Documentation]                         Verify data NACK (EIO) occurs
    PHiLIP.write and execute                i2c.mode.nack_data  1
    API Call Should Error                   I2C Read Reg
    API Result Message Should Contain       EIO
