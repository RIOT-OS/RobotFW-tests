*** Settings ***
Library    DutDeviceIf    port=%{PORT}    baudrate=%{BAUD}    timeout=${%{HIL_CMD_TIMEOUT}}    connect_wait=${%{HIL_CONNECT_WAIT}}    parser=json

Resource    api_shell.keywords.txt
Resource    philip.keywords.txt
Resource    util.keywords.txt

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

Measure Timer Overhead
    [Arguments]    ${no}    ${method}    ${position}
    [Teardown]  Test Teardown

    API Call Should Succeed    Overhead Timer                 ${method}                      ${position}
    ${RESULT}=                 Run Keyword                    DutDeviceIf.Compress Result    data=${RESULT['data']}
    Record Property            timer-count                    ${RESULT['timer count']}
    Record Property            sample-count                   ${RESULT['sample count']}
    API Call Should Succeed    PHILIP.Read Trace
    ${RESULT}=                 DutDeviceIf.Filter Trace       trace=${RESULT['data']}        select=FALLING
    ${OVERHEAD}=               DutDeviceIf.Compress Result    ${RESULT}

    Record Property    overhead-${no}-${method}-${position}-timer    ${OVERHEAD['diff']}

Measure Timer List Overhead
    [Arguments]     ${method}  ${position}
    [Teardown]  Test Teardown

    API Call Should Succeed    Overhead Timer List            ${method}  ${position}
    API Call Should Succeed    PHILIP.Read Trace
    ${RESULT}=                 DutDeviceIf.Filter Trace       trace=${RESULT['data']}        select=FALLING
    ${OVERHEAD}=               DutDeviceIf.Compress Result    ${RESULT}

    Record Property     00-overhead-${position}-${method}    ${OVERHEAD['diff']}


Measure Timer Now Overhead
    [Teardown]  Test Teardown

    API Call Should Succeed    Overhead Timer Now
    API Call Should Succeed    PHILIP.Read Trace
    ${RESULT}=                 DutDeviceIf.Filter Trace         trace=${RESULT['data']}    select=FALLING
    ${OVERHEAD}=               DutDeviceIf.Compress Result      ${RESULT}
    Record Property            overhead-01-timer-now            ${OVERHEAD['diff']}

Measure GPIO Overhead
    [Teardown]  Test Teardown

    Run Keyword  PHILIP.Write and Execute  tmr.mode.trig_edge  1

    API Call Should Succeed    Overhead GPIO
    API Call Should Succeed    PHILIP.Read Trace

    # ${RESULT}=                 DutDeviceIf.Filter Trace                   trace=${RESULT['data']}     select=FALLING
    ${GPIO_OVERHEAD}=          DutDeviceIf.Compress Result                ${RESULT['data']}
    Record Property            overhead-00-gpio                              ${GPIO_OVERHEAD['diff']}

Set ${count} Timers
    [Documentation]            Run the list operations benchmark
    [Teardown]  Test Teardown

    API Call Should Succeed    DutDeviceIf.List Operation       ${count}
    API Call Should Succeed    PHILIP.Read Trace
    ${PHILIP_RES}=             DutDeviceIf.Filter Trace         ${RESULT['data']}    select=FALLING
    ${RESULT}=                 DutDeviceIf.Compress Result      ${PHILIP_RES}

    Record Property            ${count}-timer-trace             ${RESULT['diff']}

*** Test Cases ***
Measure GPIO
    [Teardown]  Run Keywords  PHILIP Reset
    Repeat Keyword  20  Measure GPIO Overhead

Measure Overhead TIMER_NOW
    [Teardown]  Run Keywords  PHILIP Reset
    Repeat Keyword  20  Measure Timer Now Overhead

Measure Overhead Set List
    [Teardown]  Run Keyword     PHILIP Reset
    FOR  ${n}  IN RANGE  25
        Repeat Keyword  1 times  Measure Timer List Overhead     set     ${n + 1}
    END

Measure Overhead Remove List
    [Teardown]  Run Keyword     PHILIP Reset
    FOR  ${n}  IN RANGE  25
        Repeat Keyword  1 times  Measure Timer List Overhead     remove     ${n + 1}
    END
