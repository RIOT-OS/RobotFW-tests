*** Settings ***
Documentation       Tests clock stretching.

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
Read Register with clock stretching
    [Documentation]             Verify DUT functions with clock stretching.
    API Call Should Succeed     I2C Read Regs       reg=2   leng=10
    Set Test Variable           ${accepted_vals}    ${RESULT['data']}
    API Call Should Succeed     PHiLIP.write_reg    i2c.mode.init          0
    API Call Should Succeed     PHiLIP.write_reg    i2c.clk_stretch_delay  0
    API Call Should Succeed     PHiLIP.Execute Changes
    API Call Should Succeed     I2C Read Regs       reg=2   leng=10
    Should Be Equal             ${RESULT['data']}  ${accepted_vals}
    API Call Should Succeed     PHiLIP.write_reg    i2c.mode.init          0
    API Call Should Succeed     PHiLIP.write_reg    i2c.clk_stretch_delay  1000
    API Call Should Succeed     PHiLIP.Execute Changes
    API Call Should Succeed     I2C Read Regs       reg=2   leng=10
    Should Be Equal             ${RESULT['data']}  ${accepted_vals}
    API Call Should Succeed     PHiLIP.write_reg    i2c.mode.init          0
    API Call Should Succeed     PHiLIP.write_reg    i2c.clk_stretch_delay  16000
    API Call Should Succeed     PHiLIP.Execute Changes
    API Call Should Succeed     I2C Read Regs       reg=2   leng=10
    Should Be Equal             ${RESULT['data']}  ${accepted_vals}
