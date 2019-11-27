*** Settings ***
Documentation       Verify basic functionality of the Periph Timer API.

# reset application and check DUT has correct firmware, skip all tests on error
Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Firmware Should Match
# reset application before running any test
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell

# import libs and keywords
Resource            api_shell.keywords.txt
Resource            periph_timer.keywords.txt

# add default tags to all tests
Force Tags          periph_timer

*** Test Cases ***
Timer Init Should Succeed
    [Documentation]             Verify timer_init return code
    API Call Should Succeed     Timer Init  freq=%{HIL_PERIPH_TIMER_HZ}

Timer Read Should Succeed
    [Documentation]             Verify timer_read returns a value
    API Call Should Succeed     Timer Init  freq=%{HIL_PERIPH_TIMER_HZ}
    API Call Should Succeed     Timer Read

Timer Clear Should Succeed
    [Documentation]             Verify timer_clear return code
    API Call Should Succeed     Timer Init  freq=%{HIL_PERIPH_TIMER_HZ}
    API Call Should Succeed     Timer Clear

Timer Set Should Succeed
    [Documentation]             Verify timer_set return code
    API Call Should Succeed     Timer Init  freq=%{HIL_PERIPH_TIMER_HZ}
    API Call Should Succeed     Timer Set  ticks=${10000}
