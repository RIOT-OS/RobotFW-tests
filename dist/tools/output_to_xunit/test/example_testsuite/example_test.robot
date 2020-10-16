*** Settings ***
Library     TestData

Resource    util.keywords.txt

*** Keywords ***
Call Library
    [Arguments]         ${call}
    ${RESULT}=          Run Keyword  ${call}
    Set Suite Variable  ${RESULT}

Predefined Keyword
    Call Library        TestData.Get Long String
    Record Property     LONG_STRING    ${RESULT}

*** Test Cases ***
Passed Testcase
    Pass Execution  TEST PASSED
Failed Testcase
    Fail            TEST FAILED
Pass Non-Critical Testcase
    Set Tags        non-critical
Fail Non-Critical Testcase
    Set Tags        non-critical
    Fail
Record Empty value
    Call Library        TestData.Get Empty
    Record Property     LONG_STRING    ${RESULT}
Record String value
    Call Library        TestData.Get Long String
    Record Property     LONG_STRING    ${RESULT}
Record Long List
    Call Library        TestData.Get Long List
    Record Property     LONG_LIST      ${RESULT}
Record Long Dict
    Call Library        TestData.Get Long Dict
    Record Property     LONG_DICT      ${RESULT}
Record using keyword    Predefined Keyword
