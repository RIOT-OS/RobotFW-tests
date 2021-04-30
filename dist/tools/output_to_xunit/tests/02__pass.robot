*** Settings ***
Documentation       Simple robot test that passes.

Force Tags          pass

*** Test Cases ***
Assert Test Passes
    Should Be Equal  True  True

Assert Test Passes Again
    Should Be Equal  True  True
