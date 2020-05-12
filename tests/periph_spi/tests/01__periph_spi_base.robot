*** Settings ***
Documentation       Verify basic functionality of the periph SPI API.

Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell
...                                 API Firmware Should Match
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell
...                                 SPI Init Should Succeed

Resource            periph_spi.keywords.txt
Resource            api_shell.keywords.txt

Variables           test_vars.py

Force Tags          periph  spi

*** Test Cases ***
Acquire and Release Should Succeed
    [Documentation]             Verify SPI acquire and release API calls.
    SPI Acquire Should Succeed  0  100k
    SPI Release Should Succeed

Acquire after Release Should Succeed
    [Documentation]             Verify acquiring an SPI bus after release.
    SPI Acquire Should Succeed  0  100k
    SPI Release Should Succeed
    SPI Acquire Should Succeed  0  100k

Double Acquire Should Timeout
    [Documentation]             Verify that acquiring a locked SPI bus blocks.
    SPI Acquire Should Succeed  0  100k
    SPI Acquire Should Timeout  0  100k
