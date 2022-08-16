# ztimer_benchmarks test

This application provides a wrapper for benchmarking ztimer via the RIOT shell.
The application uses the GPIO pin defined by HIL_DUT_IC to trigger the IC pin on
PHiLIP.

Running this test on `samr21-xpro` with robot framework is done with this command:
`BOARD=samr21-xpro PORT=/dev/ttyACM0 PHILIP_PORT=/dev/ttyACM1 HIL_DUT_IC_PORT=0 HIL_DUT_IC_PIN=19 make robot-test`

where HIL_DUT_IC_PORT and HIL_DUT_IC_PIN are the RIOT specific pin identifiers
of the DUT pin that is connected to PHiLIPs IC pin. Consult `dist/etc/conf/` for
specific board configuration
