*** Settings ***
Documentation       Basic tests to verify functionality of the Xtimer API.

# reset application and check DUT has correct firmware, skip all tests on error
Suite Setup         Run Keywords    Reset Application
...                                 API Firmware Should Match
# reset application and check DUT is up again befor every test case
Test Setup          Run Keywords    Reset Application
...                                 API Firmware Should Match

# import libs and keywords
Library             Xtimer  port=%{PORT}  baudrate=%{BAUD}  timeout=${10}
Resource            api_shell.keywords.txt
Resource            riot_base.keywords.txt

# add default tags to all tests
Force Tags          xtimer

*** Test Cases ***
Xtimer Now Should Succeed
    [Documentation]             Verify xtimer_now() API call.
    API Call Should Succeed     Xtimer Now

Xtimer Values Should Increase
    [Documentation]             Compare two xtimer values (t1, t2) and verify
    ...                         that they increase (t2 > t1).
    API Call Should Succeed     Xtimer Now
    ${t1}=                      API Get Last Result As Integer
    API Call Should Succeed     Xtimer Now
    ${t2}=                      API Get Last Result As Integer
    Should Be True              ${t2} > ${t1}
