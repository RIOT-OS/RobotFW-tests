*** Settings ***
Documentation       Verify that the measured times are within a certain window

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
Resource            util.keywords.txt

Variables           test_vars.py

Force Tags          periph  spi


*** Keywords ***
SPI Clock Speed Check Setup
    [Documentation]     Main routine to verify the clock speed
    [Arguments]         ${clk_speed_string}  ${clk_speed_value}  ${size}
    Set Test Variable                   ${size}
    PHiLIP.write and execute            spi.mode.if_type    4
    SPI Acquire Should Succeed          0                   ${clk_speed_string}
    SPI Transfer Bytes Should Succeed   cont=0              in_len=${size}
    API Call Should Succeed             PHiLIP.Read Reg     sys.sys_clk
    Set Test Variable                   ${sys_clk}          ${RESULT['data']}
    ${frame_stats}=                     PHiLIP.Get Spi Clk Frame Stats

    Set Test Variable                   ${comparison}
    ...                                     ${spi_speed_comparison(${clk_speed_value}, ${frame_stats}, ${sys_clk}, ${size})}

    Record Property     expected_freq   ${clk_speed_value}
    Record Property     measured_freq   ${comparison['measured_freq']}
    Record Property     diff_pct        ${comparison['difference_percentage']}
    Record Property     byte_count      ${size}

    IF   not ${comparison['pass']}
        Run Keyword
        ...  Fail Test  ${comparison['difference_percentage']}  ${comparison['measured_freq']}
    ELSE IF  ${comparison['warn']}
        Run Keyword And Warn On Failure
        ...  Fail Test  ${comparison['difference_percentage']}  ${comparison['measured_freq']}
    ELSE
        Pass Execution  Measured frequency is ${comparison['measured_freq']}Hz and within range.
    END


Fail Test
    [Documentation]     Fails the test and gives a unified message
    [Arguments]         ${difference_percentage}  ${measured_freq}
    Fail                Clock speed is ${difference_percentage}% off. Measured frequency: ${measured_freq}Hz


*** Test Cases ***
Clock Speed 100k Should Succeed 1 Byte
    [Documentation]  Checks the clock speed for 100kHz with 1 bytes
    SPI Clock Speed Check Setup    100k     100000    1

Clock Speed 100k Should Succeed 2 Byte
    [Documentation]  Checks the clock speed for 100kHz with 2 bytes
    SPI Clock Speed Check Setup    100k     100000    2

Clock Speed 100k Should Succeed 8 Byte
    [Documentation]  Checks the clock speed for 100kHz with 8 bytes
    SPI Clock Speed Check Setup    100k     100000    8

Clock Speed 400k Should Succeed 1 Byte
    [Documentation]  Checks the clock speed for 400kHz with 1 bytes
    SPI Clock Speed Check Setup    400k     400000    1

Clock Speed 400k Should Succeed 2 Byte
    [Documentation]  Checks the clock speed for 400kHz with 2 bytes
    SPI Clock Speed Check Setup    400k     400000    2

Clock Speed 400k Should Succeed 8 Byte
    [Documentation]  Checks the clock speed for 400kHz with 8 bytes
    SPI Clock Speed Check Setup    400k     400000    8

Clock Speed 1M Should Succeed 1 Byte
    [Documentation]  Checks the clock speed for 1MHz with 1 bytes
    SPI Clock Speed Check Setup      1M    1000000    1

Clock Speed 1M Should Succeed 2 Byte
    [Documentation]  Checks the clock speed for 1MHz with 2 bytes
    SPI Clock Speed Check Setup      1M    1000000    2

Clock Speed 1M Should Succeed 8 Byte
    [Documentation]  Checks the clock speed for 1MHz with 8 bytes
    SPI Clock Speed Check Setup      1M    1000000    8

Clock Speed 5M Should Succeed 1 Byte
    [Documentation]  Checks the clock speed for 5MHz with 1 bytes
    SPI Clock Speed Check Setup      5M    5000000    1

Clock Speed 5M Should Succeed 2 Byte
    [Documentation]  Checks the clock speed for 5MHz with 2 bytes
    SPI Clock Speed Check Setup      5M    5000000    2

Clock Speed 5M Should Succeed 8 Byte
    [Documentation]  Checks the clock speed for 5MHz with 8 bytes
    SPI Clock Speed Check Setup      5M    5000000    8
