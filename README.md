# RIOT HIL Tests

A suite of applications and scripts to run hardware-in-the-loop (HIL) tests for
the RIOT-OS.

## Prerequisites

* RIOT-OS development environment, i.e., compilers and tools
* Python 3.x and `pip3` to run tests
* Python dependencies, to install:
```
pip3 install -r dist/robotframework/requirements.txt
```

## Getting started

For the impatient developer or user familiar with RIOT-OS follow these steps
for a quick start:

1. Clone this repository recursively:
```
git clone --recursive ...
```
2. Connect your favourite board
3. From the root folder of this repository run
```
BOARD=<name> make -C tests/xtimer_cli flash robot-test
```

## Further Information

All test applications, scripts and tools here should follow the guide lines
specified in the RIOT RDM *Guidelines for Write Firmware to Expose RIOT APIs*,
(see [draft]).

The tests scripts utilise format and syntax of the [RobotFramework]. Additional
files related to RF can be found in `dist/robotframework`, this includes common
keywords and custom libraries.

Besides the applications that are written in C, it is recommend to use Python
to write tools or interfaces related to running tests. That allows for simple
integration with the RF test scripts (and re-usability in general).

## Testing and Changing RIOT Versions

To test a RIOT version go into the RIOT subdirectory and change to the
desired state.  Alternatively, the `RIOTBASE=<path_to_riot_directory>` can
be used.

## Build System Integration

The (HIL) tests, i.e., the RF test scripts, utilise and integrate with the RIOT
build system. For instance, it uses existing `make` targets like `flash` and
environment variables such as `BOARD` and `PORT`. Further, new targets are
introduced namely `robot-test` to specifically trigger RF based HIL testing
without interfering with the existing `test` target in RIOT and `robot-clean`
to clean the build folder and hence, enable the tests to be rerun.

Additional `robot` flags can be added with `ROBOT_EXTRA_ARGS`.
This can be useful for fine-tuning robot performance while taking advantage of
the build system.
For example, selecting a single test suite for a test binary:
```
ROBOT_EXTRA_ARGS="-s mytestsuitename" make robot-test
```

[draft]: https://github.com/RIOT-OS/RIOT/pull/10624
[RobotFramework]: https://robotframework.org
