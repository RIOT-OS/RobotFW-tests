*** Settings ***
Documentation       Verify basic functionality of the Xtimer API.

# reset application and check DUT has correct firmware, skip all tests on error
Suite Setup         Run Keywords    RIOT Reset
...                                 API Firmware Should Match
# reset application before running any test
Test Setup          Run Keywords    RIOT Reset
...                                 API Sync Shell

# import libs and keywords
Library             Xtimer  port=%{PORT}  baudrate=%{BAUD}  timeout=${%{CMD_TIMEOUT}}
Resource            api_shell.keywords.txt
Resource            riot_base.keywords.txt

# add default tags to all tests
Force Tags          xtimer

*** Test Cases ***
Xtimer Now Should Succeed
    [Documentation]             Verify xtimer_now() API call.
    API Call Should Succeed     Xtimer Now

Xtimer Values Should Increase
    [Documentation]             Verify xtimer values are monotonously increasing.
    API Call Should Succeed     Xtimer Now
    ${t1}=                      API Get Last Result As Integer
    API Call Should Succeed     Xtimer Now
    ${t2}=                      API Get Last Result As Integer
    Should Be True              ${t2} > ${t1}
