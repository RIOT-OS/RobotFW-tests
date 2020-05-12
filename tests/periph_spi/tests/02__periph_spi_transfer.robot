*** Settings ***
Documentation       Verify functionality of transfering bytes and regs of the periph SPI API.

Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell
...                                 API Firmware Should Match
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell
...                                 SPI Init Should Succeed
Test Teardown       Run Keywords    SPI Release Should Succeed

Resource            periph_spi.keywords.txt
Resource            api_shell.keywords.txt

Variables           test_vars.py

Force Tags          periph  spi


*** Test Cases ***

Transfer Single Bytes Should Succeed
    [Documentation]  Sends a single byte two times using the spi_transfer_byte function.
    ...              Once with the continue parameter and once without.
    SPI Acquire Should Succeed          0  100k
    SPI Transfer Byte Should Succeed   cont=1   out=41
    Should Be Equal                     ${RESULT['data']}   ${VAL_1}
    SPI Transfer Byte Should Succeed   cont=0   out=1
    Should Be Equal                     ${RESULT['data']}   ${VAL_2}


Transfer Multiple Bytes Should Succeed
    [Documentation]  Sends 1, 2 and 16 bytes using the spi_transfer_bytes function.
    ...              Uses multiple settings of the function parameters:
    ...              cont is set and not set
    ...              out has 1, 16 and none values
    SPI Acquire Should Succeed          0  100k
    SPI Transfer Bytes Should Succeed   cont=1   in_len=1   out=41
    Should Be Equal                     ${RESULT['data']}   ${VAL_1}
    SPI Transfer Bytes Should Succeed   cont=0   in_len=2
    Should Be Equal                     ${RESULT['data']}   ${VAL_3}
    SPI Transfer Bytes Should Succeed   cont=0   in_len=16  out=${VAL_4}
    Should Be Equal                     ${RESULT['data']}   ${VAL_5}


Transfer Single Reg Should Succeed
    [Documentation]  Send a byte to a given register and reads it back using the
    ...              spi_transfer_reg function
    SPI Acquire Should Succeed          0  100k
    SPI Transfer Reg Should Succeed     reg=41   out=5
    Should Be Equal                     ${RESULT['data']}   ${VAL_2}
    SPI Transfer Reg Should Succeed     reg=41   out=0
    Should Be Equal                     ${RESULT['data']}   ${VAL_6}

Transfer Multiple Regs Should Succeed
    [Documentation]  Sends 1, 2 and 16 bytes strarting from a given register
    ...              using the spi_transfer_regs function
    SPI Acquire Should Succeed          0  100k
    SPI Transfer Regs Should Succeed    reg=41   in_len=1   out=5
    Should Be Equal                     ${RESULT['data']}   ${VAL_2}
    SPI Transfer Regs Should Succeed    reg=42   in_len=2   out=6 7
    Should Be Equal                     ${RESULT['data']}   ${VAL_7}
    SPI Transfer Regs Should Succeed    reg=41   in_len=16  out=${VAL_4}
    Should Be Equal                     ${RESULT['data']}   ${VAL_8}
