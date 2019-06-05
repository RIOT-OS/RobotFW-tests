*** Settings ***
Documentation       Data driven tests to verify the i2c_write_regs call.

Suite Setup         Run Keywords    Reset DUT and PHILIP
...                                 API Firmware Should Match
Test Setup          Run Keywords    Reset DUT and PHILIP
...                                 API Firmware Should Match
...                                 I2C Acquire
Test Teardown       I2C Release
Test Template       I2C Write Bytes To Register Should Succeed

Resource            periph_i2c.keywords.txt
Variables           test_vars.py

Force Tags          periph  i2c

*** Test Cases ***          REG             DATA        LENGTH
One Byte Should Succeed     ${I2C_UREG}     ${VAL_1}    1
Two Bytes Should Succeed    ${I2C_UREG}     ${VAL_2}    2
Ten Bytes Should Succeed    ${I2C_UREG}     ${VAL_10}   10
