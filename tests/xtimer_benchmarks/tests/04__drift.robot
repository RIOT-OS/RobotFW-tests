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

Force Tags     long

*** Keywords ***
Measure Drift
    [Documentation]            Run the drift simple benchmark
    [Arguments]                ${duration}
    [Teardown]                 Run Keywords                          PHILIP Reset         API Sync Shell
    FOR                        ${i}                                  IN RANGE             5
    API Call Should Succeed    Drift                                 ${duration}
    Record Property            dut-result-${duration}-repeat-${i}    ${RESULT['data']}

    API Call Should Succeed    PHILIP.Read Trace
    ${PHILIP_RES}=             DutDeviceIf.Filter Trace                 ${RESULT['data']}    select=FALLING
    ${RESULT}=                 DutDeviceIf.Compress Result              ${PHILIP_RES}
    Record Property            philip-result-${duration}-repeat-${i}    ${RESULT['diff']}
    PHILIP Reset
    END

*** Test Cases ***
Measure Drift Template
    [Teardown]  Run Keywords  PHILIP Reset
    [Template]  Measure Drift
    1000000
    15000000
    30000000
    45000000
    59000000
    # 60000000
