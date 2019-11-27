# Robot Framework Automated Hardware Tests

These are tests that are run with Robot Framework and involve shell
interactions and usually use reference hardware.

## Glossary

DUT - Device Under Test
[PHiLIP](https://github.com/riot-appstore/PHiLIP) - The reference hardware to
test against

## make overrides
Variable         | Description                                     | Example(s)                          | Required | Default
-----------------|-------------------------------------------------|-------------------------------------|----------|-------------------------
BOARD            | The name of the DUT board                       | nucleo-f030r8, samr21-xpro          | Yes      |
PORT             | The serial port of the DUT                      | /dev/ttyACM0, /dev/ttyUSB1          | No       | _make system handles it_
BAUD             | The baudrate of the DUT serial connection       | 115200, 9600                        | No       | _make system handles it_
PHILIP_PORT      | The serial port of PHiLIP                       | /dev/ttyACM0                        | Yes      |
HIL_CMD_TIMEOUT  | The time (s) for a command to respond           | 0.5                                 | No       | 1
HIL_RESET_WAIT   | The time (s) to wait after a reset              | 3                                   | No       | 3
HIL_CONNECT_WAIT | The time (s) to wait after connecting to serial | 5                                   | No       | 0
RESET            | The command used to reset the device            | 'python3 -m philip_pal --dut_reset' | No       | _make system handles it_
RESET_FLAGS      | Flags for the reset command                     | /dev/ttyACM0, ${PHILIP_PORT}        | No       | _make system handles it_

The `HIL_RESET_WAIT` is useful for speeding up tests if a longer reset wait period
is not needed like in the case of using the PHiLIP reset. Then `HIL_RESET_WAIT`
can be 0. For some boards, such as the Arduino, the bootloader is activated
after startup and `HIL_RESET_WAIT` must be used to wait until the bootloader times
out before sending.

The `HIL_CONNECT_WAIT` is needed when opening a serial connection causes reset.
On boards such as the Arduino a connection wait of 4 seconds is needed so
the bootloader times out.
