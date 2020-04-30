*** Settings ***
Documentation       Verify functionality of all 4 modes of the periph SPI API.

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

*** Variables ***
${hw_update_time}   0.1


*** Keywords ***
SPI Modes Transfer Bytes Should Succeed
    [Documentation]     Initializes the SPI on both sides to the given mode
    ...                 sends bytes and reads them back.
    [Arguments]         ${mode}  ${cpol}  ${cpha}
    API Call Should Succeed            PHiLIP.Write Reg       spi.mode.cpol    ${cpol}
    API Call Should Succeed            PHiLIP.Write Reg       spi.mode.cpha    ${cpha}
    API Call Should Succeed            PHiLIP.Write Reg       spi.mode.init       0
    API Call Should Succeed            PHiLIP.Execute Changes
    Sleep                              ${hw_update_time}

    SPI Acquire Should Succeed         ${mode}             100k
    SPI Transfer Bytes Should Succeed  cont=0              in_len=5   out=7 11 2 14 5
    Should Be Equal                    ${RESULT['data']}   ${VAL_9}
    SPI Transfer Bytes Should Succeed  cont=0              in_len=5   out=7 1 2 3 4
    Should Be Equal                    ${RESULT['data']}   ${VAL_10}


*** Test Cases ***
Transfer Bytes Mode 0 Should Succeed
    [Documentation]   Transfers Bytes in SPI mode 0 (cpol=0 and cpha=0)
    SPI Modes Transfer Bytes Should Succeed    0  0  0


Transfer Bytes Mode 1 Should Succeed
    [Documentation]   Transfers Bytes in SPI mode 1 (cpol=0 and cpha=1)
    SPI Modes Transfer Bytes Should Succeed    1  0  1


Transfer Bytes Mode 2 Should Succeed
    [Documentation]   Transfers Bytes in SPI mode 2 (cpol=1 and cpha=0)
    SPI Modes Transfer Bytes Should Succeed    2  1  0


Transfer Bytes Mode 3 Should Succeed
    [Documentation]   Transfers Bytes in SPI mode 3 (cpol=1 and cpha=1)
    SPI Modes Transfer Bytes Should Succeed    3  1  1



