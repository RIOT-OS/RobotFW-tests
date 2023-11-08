*** Settings ***
Documentation       Verify if board to philip reset wiring is correct.

Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Firmware Should Match
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell

Resource            api_shell.keywords.txt
Resource            periph_gpio.keywords.txt

Force Tags          periph  gpio

*** Test Cases ***
Verify Reset Pin Is Connected
    [Documentation]             Lock the dut and try run another API Call.
    ...                         It should not succeed until the dut gets reset.
    Sleep                       1
    API Call Should Timeout     lock
    API Call Should Timeout     get_metadata
    Run Keyword                 RIOT Reset
    API Sync Shell
