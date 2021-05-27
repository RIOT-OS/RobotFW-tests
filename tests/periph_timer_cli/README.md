# periph timer test

This application provides a wrapper for testing the periph_timer api via the
RIOT shell. To capture fast low level timing events some functions provide
additional parameters to signal events over gpio pins e.g. before calling a
timer function or on ISR callback execution.
While this test can be run manually and with any external measurement equipment,
it is recommended to use it together with the provided robot test on a PHiLIP-
based HIL setup.
Consult the 'help' shell command for available actions.

Running this test with robot framework is done with this command:
`BOARD=whatever PORT=/dev/ttyACM0 PHILIP_PORT=/dev/ttyACM1 DEBUG0_PORT=1 DEBUG0_PIN=3 make robot-test`

Where DEBUG0_PORT and DEBUG0_PIN are the RIOT specific pin identifiers of the
DUT pin that is connected to PHiLIPs DEBUG0 pin.
