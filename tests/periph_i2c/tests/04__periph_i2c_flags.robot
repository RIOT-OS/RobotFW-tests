*** Settings ***
Documentation       Tests split frame i2c flags.

Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Firmware Should Match
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell
...                                 I2C Acquire
Test Teardown       I2C Release

Resource            periph_i2c.keywords.txt
Resource            api_shell.keywords.txt
Resource            riot_base.keywords.txt
Variables           test_vars.py

Force Tags          periph  i2c

*** Test Cases ***
Read Byte with NOSTOP Flag Should Succeed
    [Documentation]                         Verify DUT does not lockup if read_byte with NOSTOP
    API Call Should Succeed                 I2C Read Byte  flag=${I2C_FLAG_NOSTOP}

Read Bytes with NOSTOP Flag Should Succeed
    [Documentation]                         Verify DUT does not lockup if read_bytes with NOSTOP
    API Call Should Succeed                 I2C Read Bytes  flag=${I2C_FLAG_NOSTOP}

Repeated Start Read Bytes 0 Should Succeed
    [Documentation]                         Verify DUT does not lockup on repeated start read
    PHiLIP.write reg                        user_reg  0
    API Call Should Succeed                 I2C Read Bytes  leng=2  flag=${I2C_FLAG_NOSTOP}
    API Call Should Error                   I2C Read Bytes  leng=2
    API Call Should Succeed                 I2C Read Bytes  leng=2  flag=${I2C_FLAG_NOSTART}

Repeated Start Read Bytes 0xFF Should Succeed
    [Documentation]                         Verify DUT does not lockup on repeated start read
    PHiLIP.write reg                        user_reg  255
    API Call Should Succeed                 I2C Read Bytes  leng=2  flag=${I2C_FLAG_NOSTOP}
    API Call Should Error                   I2C Read Bytes  leng=2
    API Call Should Succeed                 I2C Read Bytes  leng=2  flag=${I2C_FLAG_NOSTART}

Read Bytes By Frame Should Succeed
    [Documentation]                         Read bytes frame by frame
    API Call Should Succeed                 I2C Read Bytes  flag=${I2C_FLAG_NOSTOP}
    API Call Should Succeed                 I2C Read Bytes  flag=${I2C_FLAG_NOSTARTSTOP}
    API Call Should Succeed                 I2C Read Bytes  flag=${I2C_FLAG_NOSTART}

Write Bytes By Frame Should Succeed
    [Documentation]                         Write bytes frame by frame
    API Call Should Succeed                 I2C Write Bytes  flag=${I2C_FLAG_NOSTOP}
    API Call Should Succeed                 I2C Write Bytes  flag=${I2C_FLAG_NOSTARTSTOP}
    API Call Should Succeed                 I2C Write Bytes  flag=${I2C_FLAG_NOSTART}
