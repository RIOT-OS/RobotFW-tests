# README

Parses `output.xml` from a RobotFramework result to a xUnit style format. The parsing is done by the `output_to_xunit.py` script. Run `output_to_xunit.py -h` to get the usage info.

## Testing

```
# Run tests and generate output.xml
robot --outputdir build --pythonpath "test:../../robotframework/res/" --noncritical non-critical --noncritical warn-if-failed test/example_testsuite/

# Parse output and generate xunit.xml in the current directory (default)
python3 output_to_xunit.py build/output.xml

# Specifying custom output directory/name
python3 output_to_xunit.py --output build/test_xunit.xml build/output.xml
```

NOTE: `Test Failed Testcase` and `Test Fail Non-Critical Testcase` is supposed to fail.

## Schema Validation

TODO


