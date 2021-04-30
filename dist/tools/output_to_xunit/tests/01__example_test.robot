*** Settings ***
Library     TestData.py

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

Fail With Nice Message
    Should Not Be Equal  True  True  This is my failure message, much better that True==True  False

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

Assert Test Fails Warning
    Run Keyword And Warn On Failure  Fail
    ...  It is OK to, this will show PASS and have a WARN tag here or there

Skip Test
    Skip  I am skipping this test

Skip If Test
    Skip If  True  I am skipping this test

Do Not Skip Test
    Skip If  False  I should run this test
