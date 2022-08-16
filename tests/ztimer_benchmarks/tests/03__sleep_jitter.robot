*** Settings ***
Library    DutDeviceIf    port=%{PORT}    baudrate=%{BAUD}    timeout=${%{HIL_CMD_TIMEOUT}}    connect_wait=${%{HIL_CONNECT_WAIT}}    parser=json

Resource    api_shell.keywords.txt
Resource    philip.keywords.txt

Suite Setup    Run Keywords
...            RIOT Reset
...            PHILIP Reset
...            API Firmware Data Should Match
Test Setup     Run Keywords
...            PHILIP Reset
...            API Sync Shell

*** Keywords ***
Test Teardown
    Run Keyword If  '${KEYWORD_STATUS}' != 'PASS'     RIOT Reset
    PHILIP Reset

Measure Sleep Jitter With ${timer_count} Timers
    [Documentation]            Run the sleep jitter benchmark
    [Teardown]                 Test Teardown

    API Call Should Succeed    Sleep Jitter                     ${timer_count}
    ${RESULT}=                 DutDeviceIf.Compress Result      ${RESULT['data']}
    Record Property            timer-interval                   ${RESULT['timer-interval']}
    Record Property            dut-${timer_count}-start-time    ${RESULT['start-time']}
    Record Property            dut-${timer_count}-wakeup-time   ${RESULT['wakeups']}

    API Call Should Succeed    PHILIP.Read Trace
    ${RESULT}=                 DutDeviceIf.Compress Result      ${RESULT['data']}
    Record Property            hil-${timer_count}-start-time    ${RESULT['time'][0]}
    Record Property            hil-${timer_count}-wakeup-time   ${RESULT['time'][1:-1]} # exclude start event

*** Test Cases ***
Measure Sleep Jitter With Increasing Timers
    FOR  ${n}  IN RANGE  10
        Repeat Keyword  3 Times     Measure Sleep Jitter With ${n + 1} Timers
    END
