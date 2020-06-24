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
    PHiLIP.write and execute            spi.mode.if_type    3
    SPI Acquire Should Succeed          0  ${clk_speed_string}
    SPI Transfer Bytes Should Succeed   cont=0   in_len=${size}
    API Call Should Succeed             PHiLIP.Read Reg         sys.sys_clk
    ${sys_clk} =  Set Variable          ${RESULT['data']}
    API Call Should Succeed             PHiLIP.Read Reg         spi.byte_ticks
    ${spi_speed_limits} =  Set Variable  
    ...  ${spi_speed_limits(${clk_speed_value}, ${sys_clk}, ${size})}
    Should Be True  
    ...  ${spi_speed_limits}[0] < ${${RESULT['data']} * ${size}} and ${spi_speed_limits}[1] > ${${RESULT['data']} * ${size}}
    ...  Time should be between ${convert_ticks_to_us(${spi_speed_limits[0]},${sys_clk})}us and ${convert_ticks_to_us(${spi_speed_limits[1]},${sys_clk})}us but was ${convert_ticks_to_us(${RESULT['data']} * ${size}, ${sys_clk})}us


*** Test Cases ***
Clock Speed 100k Should Succeed
    [Documentation]  Checks the clock speed for 100kHz
    SPI Clock Speed Check    100k     100000    1

Clock Speed 100k Should Succeed 8 Byte
    [Documentation]  Checks the clock speed for 100kHz
    SPI Clock Speed Check    100k     100000    8

Clock Speed 400k Should Succeed
    [Documentation]  Checks the clock speed for 400kHz
    SPI Clock Speed Check    400k     400000    1

Clock Speed 400k Should Succeed 8 Byte
    [Documentation]  Checks the clock speed for 400kHz
    SPI Clock Speed Check    400k     400000    8

Clock Speed 1M Should Succeed
    [Documentation]  Checks the clock speed for 1MHz
    SPI Clock Speed Check      1M    1000000    1

Clock Speed 1M Should Succeed 8 Byte
    [Documentation]  Checks the clock speed for 1MHz
    SPI Clock Speed Check      1M    1000000    8

Clock Speed 5M Should Succeed
    [Documentation]  Checks the clock speed for 5MHz
    SPI Clock Speed Check      5M    5000000    1

Clock Speed 5M Should Succeed 8 Byte
    [Documentation]  Checks the clock speed for 5MHz
    SPI Clock Speed Check      5M    5000000    8

Clock Speed 10M Should Succeed
    [Documentation]  Checks the clock speed for 10MHz
    SPI Clock Speed Check     10M   10000000    1

Clock Speed 10M Should Succeed 8 Byte
    [Documentation]  Checks the clock speed for 10MHz
    SPI Clock Speed Check     10M   10000000    8
