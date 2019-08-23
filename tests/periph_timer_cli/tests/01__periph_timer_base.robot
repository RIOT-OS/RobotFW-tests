*** Settings ***
Documentation       Verify basic functionality of the Periph Timer API.

# reset application and check DUT has correct firmware, skip all tests on error
Suite Setup         Run Keywords    PHiLIP.DUT Reset
...                                 API Firmware Should Match
# reset application before running any test
Test Setup          Run Keywords    PHiLIP.DUT Reset
...                                 API Sync Shell

# import libs and keywords
Library             PeriphTimer  port=%{PORT}  baudrate=%{BAUD}  timeout=${%{CMD_TIMEOUT}}
Resource            api_shell.keywords.txt
Resource            riot_base.keywords.txt
Resource            philip.keywords.txt

# add default tags to all tests
Force Tags          periph_timer

*** Variables ***
${ACCEPTABLE_ERROR_PERCENT}    ${2}
${DEBUG_PORT}                  %{DEBUG0_PORT}
${DEBUG_PIN}                   %{DEBUG0_PIN}

*** Keywords ***
Enable Debug GPIO Trace On Pin ${pin}
    [Documentation]           Enable tracing for a gpio debug pin
    API Call Should Succeed   PHiLIP.Write Reg  gpio[${pin}].mode.init  0
    API Call Should Succeed   PHiLIP.Write Reg  gpio[${pin}].mode.io_type  3
    API Call Should Succeed   PHiLIP.Execute Changes

Timer Set To ${ticks} And Measure
    [Documentation]           Set a timer and use PHiLIP to measure the delay
    ...                       between calling set_timer and entering the timer
    ...                       ISR callback by tracing a GPIO debug pin
    Run Keywords              PHiLIP Reset
    Run Keywords              Enable Debug GPIO Trace On Pin 0
    API Call Should Succeed   Timer Set  dev=0  chan=0  ticks=${ticks}  gpio_port=${DEBUG_PORT}  gpio_pin=${DEBUG_PIN}

Timer Init With ${freq} Hz and Set To ${ticks}
    API Call Should Succeed   Timer Init  dev=0  freq=${freq}  cbname=cb_toggle  gpio_port=${DEBUG_PORT}  gpio_pin=${DEBUG_PIN}
    API Call Should Succeed   Timer Set  dev=0  chan=0  ticks=${ticks}  gpio_port=${DEBUG_PORT}  gpio_pin=${DEBUG_PIN}

Measure Timer Delay For ${ticks}
    [Documentation]           Returns the time it took between set_timer and entering
    ...                       the timer ISR callback (measured by PHiLIP)
    Run Keyword               Timer Set To ${ticks} And Measure
    API Call Should Succeed   PHiLIP.Read Trace
    ${trace}=                 Set Variable  ${RESULT['data']}
    ${d1}=                    Evaluate  ${trace}[1][time] - ${trace}[0][time]
    [return]                  ${d1}

Check If Timer At ${freq} Hz Has Less Error Than ${perc} Percent
    [Documentation]           Checks if the timer frequency error is below the
    ...                       given percentage
    Run Keywords              PHiLIP.DUT Reset
    ${RESULT}=                Run Keyword  Timer Init  dev=0  freq=${freq}  cbname=cb_toggle  gpio_port=${DEBUG_PORT}  gpio_pin=${DEBUG_PIN}

    # skip to next frequency  if init says this frequency is not supported
    Continue For Loop If      '${RESULT['result']}' != 'Success'

    ${ticks}=                 Evaluate  ${freq} / ${2}
    ${ticks}=                 Convert To Integer  ${ticks}
    ${delay_s}=               Measure Timer Delay For ${ticks}
    ${abs_err_s}=             Evaluate  ${delay_s} - ${ticks} / ${freq}
    ${rel_error_perc}=        Evaluate  abs(${abs_err_s} / (${ticks} / ${freq}) * 100)
    Should Be True            ${rel_error_perc} < ${perc}

*** Test Cases ***
Return Codes
    [Documentation]           Basic checks to see if the return codes are valid
    Run Keyword               PHiLIP.DUT Reset
    API Call Should Succeed   Timer Init
    API Call Should Succeed   Timer Set

Timer Read Overhead
    [Documentation]           Measure how long a timer_read call takes. For now
    ...                       this is only used as benchmark without constraints
    Run Keywords              PHiLIP Reset
    Run Keywords              Enable Debug GPIO Trace On Pin 0
    ${repeat_cnt}             Set Variable  ${1000000}
    API Call Should Succeed   Timer Init  cbname=cb_toggle  gpio_port=${DEBUG_PORT}  gpio_pin=${DEBUG_PIN}
    API Call Should Succeed   Timer Read Bench  dev=0  repeat_cnt=${repeat_cnt}  gpio_port=${DEBUG_PORT}  gpio_pin=${DEBUG_PIN}
    API Call Should Succeed   PHiLIP.Read Trace
    ${trace}=                 Set Variable  ${RESULT['data']}
    ${us_per_read}=           Evaluate  (${trace}[1][time] - ${trace}[0][time]) * 1000000 / ${repeat_cnt}

Small Timer Delays
    [Template]  Timer Init With ${freq} Hz and Set To ${ticks}
    FOR    ${ticks}    IN RANGE  20  0  -1
        1000000  ${ticks}
    END

Timer Frequency Accuracy
    [Template]   Check If Timer At ${freq} Hz Has Less Error Than ${perc} Percent
    FOR    ${freq}    IN RANGE  100  1000  100
        ${freq}  ${ACCEPTABLE_ERROR_PERCENT}
    END
    FOR    ${freq}    IN RANGE  1000  10000  1000
        ${freq}  ${ACCEPTABLE_ERROR_PERCENT}
    END
    FOR    ${freq}    IN RANGE  10000  100000  10000
        ${freq}  ${ACCEPTABLE_ERROR_PERCENT}
    END
    FOR    ${freq}    IN RANGE  100000  1000000  100000
        ${freq}  ${ACCEPTABLE_ERROR_PERCENT}
    END
    FOR    ${freq}    IN RANGE  1000000  10000000  1000000
        ${freq}  ${ACCEPTABLE_ERROR_PERCENT}
    END

Timer Peak To Peak Jitter
    [Documentation]     Measure how much the measured trigger time varies when
    ...                 setting a timer for the same value multiple times
    ${freq}             Set Variable  ${1000000}
    ${ticks}            Set Variable  ${1000}
    ${diffs}            Create List
    ${maxp2pj_perc}     Set Variable  ${2}
    ${maxp2pj}          Evaluate  ${ticks} / 100 * ${maxp2pj_perc} / ${freq}

    # for a representative set of measurements this should be increased to ~100000
    ${run_cnt}          Set Variable  ${100}

    API Call Should Succeed       Timer Init  dev=0  freq=${freq}  cbname=cb_toggle  gpio_port=${DEBUG_PORT}  gpio_pin=${DEBUG_PIN}

    FOR  ${cnt}  IN RANGE  0  ${run_cnt}  1
        Run Keywords              PHiLIP Reset
        Run Keywords              Enable Debug GPIO Trace On Pin 0
        API Call Should Succeed   Timer Set  dev=0  chan=0  ticks=${ticks}  gpio_port=${DEBUG_PORT}  gpio_pin=${DEBUG_PIN}
        API Call Should Succeed   PHiLIP.Read Trace
        ${trace}=                 Set Variable  ${RESULT['data']}
        ${d1}=                    Evaluate  ${trace}[1][time] - ${trace}[0][time]
        Append To List            ${diffs}  ${d1}
    END

    Sort List                     ${diffs}
    ${min}=                       Get From List 	${diffs}  ${0}
    ${max}=                       Get From List 	${diffs}  ${run_cnt - 1}
    ${p2pj}=                      Evaluate  ${max} - ${min}
    Should Be True                ${p2pj} < ${maxp2pj}
