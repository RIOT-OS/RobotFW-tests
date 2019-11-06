*** Settings ***
Documentation       Evaluate Delays for numerous timer_set values.

# reset application and check DUT has correct firmware, skip all tests on error
Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Firmware Should Match
# reset application before running any test
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell
# set test template for data driver tests
Test Template       Measure Timer Set Delay
# import libs and keywords
Resource            api_shell.keywords.txt
Resource            periph_timer.keywords.txt

# add default tags to all tests
Force Tags          periph_timer

*** Test Cases ***              TICKS
Measure Delay of 100 Ticks      ${100}
Measure Delay of 200 Ticks      ${200}
Measure Delay of 1000 Ticks     ${1000}
Measure Delay of 2000 Ticks     ${2000}
Measure Delay of 10000 Ticks    ${10000}
Measure Delay of 20000 Ticks    ${20000}
