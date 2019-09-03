*** Settings ***
Documentation       Verify basic functionality of the Periph Timer API.

# reset application and check DUT has correct firmware, skip all tests on error
Suite Setup         Run Keywords    PHiLIP.DUT Reset
...                                 API Firmware Should Match
# reset application before running any test
Test Setup          Run Keywords    PHiLIP.DUT Reset
...                                 API Sync Shell
...                                 Timer Debug Pin

# import libs and keywords
Resource            api_shell.keywords.txt
Resource            periph_timer.keywords.txt

# add default tags to all tests
Force Tags          periph_timer

*** Test Cases ***
Timer Values Should Differ
    [Documentation]             Verify timer values are different on consecutive reads
    API Call Should Succeed     Timer Init  freq=%{PERIPH_TIMER_HZ}
    API Call Should Succeed     Timer Read
    ${t1}=                      API Result Data As Integer
    API Call Should Succeed     Timer Read
    ${t2}=                      API Result Data As Integer
    Should Be True              ${t2} != ${t1}
