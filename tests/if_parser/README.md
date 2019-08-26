# Interface Parser Test

This application enables testing of different parsers provided by `riot_pal`.

### Setup
If the [Robot Framework Setup](../../README.md) was followed then `riot_pal`
should already be installed.
Ensure the version is greater or equal to `0.3.0`.
To upgrade or install use `pip3 install --upgrade riot_pal`

## Running Tests Manually

To run tests manually simply call each command and verify the result.

### Using riot_pal For Manual Tests

The default setting has `USE_JSON_SHELL_PARSER=1` which formats the firmware
to output json.
The default parser for the `dut_pyshell` is also json.
If the parsers match then the command list should be able to autocomplete.

### Using Standard Terminal For Manual Tests

If using a standard terminal the unparsed data will be available.
