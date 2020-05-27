*** Settings ***
Documentation       Verify that the meassured times are within a certain window

Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell
...                                 API Firmware Should Match
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell
...                                 SPI Init Should Succeed
Test Teardown       Run Keywords    SPI Release Should Succeed

Resource            periph_spi.keywords.txt
Resource            api_shell.keywords.txt

Variables           test_vars.py

Force Tags          periph  spi


*** Keywords ***
SPI Clock Speed Check
    [Documentation]     Main routine to verify the clock speed
    [Arguments]         ${clk_speed_string}  ${clk_speed_value}  ${size}
    API Call Should Succeed             PHiLIP.Write Reg        spi.mode.if_type    3
    API Call Should Succeed             PHiLIP.Write Reg        spi.mode.init       0
    API Call Should Succeed             PHiLIP.Execute Changes
    SPI Acquire Should Succeed          0  ${clk_speed_string}
    SPI Transfer Bytes Should Succeed   cont=0   in_len=${size}
    API Call Should Succeed             PHiLIP.Read Reg         sys.sys_clk
    ${sys_clk} =  Set Variable          ${RESULT['data']}
    API Call Should Succeed             PHiLIP.Read Reg         spi.frame_ticks
    ${spi_speed_limits} =  Set Variable  
    ...  ${spi_speed_limits(${clk_speed_value}, ${sys_clk}, ${size})}
    Should Be True  
    ...  ${spi_speed_limits}[0] < ${RESULT['data']} and ${spi_speed_limits}[1] > ${RESULT['data']}
    ...  Number of ticks should be between ${spi_speed_limits}[0] and ${spi_speed_limits}[1] but was ${RESULT['data']}


*** Test Cases ***
Clock Speed 100k Should Succeed
    [Documentation]  Checks the clock speed for 100kHz
    SPI Clock Speed Check    100k     100000    8

Clock Speed 400k Should Succeed
    [Documentation]  Checks the clock speed for 400kHz
    SPI Clock Speed Check    400k     400000    8

Clock Speed 1M Should Succeed
    [Documentation]  Checks the clock speed for 1MHz
    SPI Clock Speed Check      1M    1000000    8

Clock Speed 5M Should Succeed
    [Documentation]  Checks the clock speed for 5MHz
    SPI Clock Speed Check      5M    5000000    8

Clock Speed 10M Should Succeed
    [Documentation]  Checks the clock speed for 10MHz
    SPI Clock Speed Check     10M   10000000    8
