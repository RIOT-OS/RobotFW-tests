*** Settings ***
Documentation       Simple robot test that passes.

Force Tags          pass

Resource            api_shell.keywords.txt

*** Keywords ***
Mock Return Success
    ${dict} =	Create Dictionary	result=Success
    [Return]  ${dict}

Mock Return Timeout
    ${dict} =	Create Dictionary	result=Timeout
    [Return]  ${dict}


Mock Key Error
    ${dict} =	Create Dictionary	foo=Success
    [Return]  ${dict}

Mock Type Error
    [Return]  Success

*** Test Cases ***
Should Mock Success
    API Call Expect  Success  Mock Return Success

Should Not Mock Success
    API Call Expect  FAIL  Mock Return Success

Should Not Mock Key Error
    API Call Expect  FAIL  Mock Key Error

Should Not Mock Type Error
    API Call Expect  FAIL  Mock Type Error

Should API Call Repeat on Timeout Success
    API Call Repeat on Timeout  Mock Return Success

Should API Call Repeat on Timeout Error
    API Call Repeat on Timeout  Mock Return Timeout
