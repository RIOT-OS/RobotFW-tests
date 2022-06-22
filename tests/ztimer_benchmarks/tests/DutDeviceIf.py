# Copyright (C) 2019 Kevin Weiss <kevin.weiss@haw-hamburg.de>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.z
"""@package PyToAPI
This module handles parsing of information from RIOT periph_gpio test.
"""
from riot_pal import DutShell
from robot.api import logger
from robot.version import get_version
from ast import literal_eval


class DutDeviceIf(DutShell):
    """Interface to the node."""

    ROBOT_LIBRARY_SCOPE = "TEST"
    ROBOT_LIBRARY_VERSION = get_version()

    def get_metadata(self):
        """Get the metadata of the firmware."""
        return self.send_cmd("get_metadata")

    def get_timer_version(self):
        """Get the version of timer used"""
        return self.send_cmd("get_timer_version")

    def overhead_gpio(self):
        """Run the GPIO toggling overhead benchmark"""
        return self.send_cmd("overhead_gpio")

    def overhead_timer_now(self):
        """Run the overhead timer_now function benchmark"""
        return self.send_cmd("overhead_timer_now")

    def overhead_timer(self, method, position):
        """Run the overhead timer benchmark"""
        return self.send_cmd("overhead_timer {} {}".format(method, position))

    def overhead_timer_list(self, method, position):
        """Run the overhead timer list benchmark"""
        return self.send_cmd("overhead_timer_list {} {}".format(method, position))

    def sleep_accuracy_timer_sleep(self, duration):
        """Run the sleep accuracy benchmark"""
        return self.send_cmd("sleep_accuracy_timer_sleep {}".format(duration))

    def sleep_accuracy_timer_set(self, duration):
        """Run the sleep accuracy benchmark"""
        return self.send_cmd("sleep_accuracy_timer_set {}".format(duration))

    def sleep_accuracy(self, call, duration):
        if call == "TIMER_SLEEP":
            return self.sleep_accuracy_timer_sleep(duration)
        elif call == "TIMER_SET":
            return self.sleep_accuracy_timer_set(duration)
        else:
            return {"result": "Error"}

    def sleep_jitter(self, timer_count):
        """Run the sleep jitter benchmark"""
        return self.cmd_extended_timeout("sleep_jitter {}".format(timer_count), 10)

    def drift(self, duration):
        """Run the drift simple benchmark"""
        return self.cmd_extended_timeout("drift {}".format(duration), 75)

    def list_operation(self, count):
        """Set N timers"""
        return self.send_cmd("list_ops {}".format(count))

    ## HELPER FUNCTIONS

    def cmd_extended_timeout(self, command, timeout):
        old_timeout = self.dev._driver._dev.timeout
        self.dev._driver._dev.timeout = timeout
        res = self.send_cmd(command)
        self.dev._driver._dev.timeout = old_timeout
        return res

    def get_dict_data(self, data, key):
        """Get data from dict"""
        assert isinstance(data, list)
        values = []
        for e in data:
            if not isinstance(e, dict):
                continue
            if key in e:
                try:
                    values.append(float(e[key]))
                except ValueError:
                    values.append(e[key])
        return values if len(values) > 1 else values[0]

    @staticmethod
    def filter_trace(trace, select: str):
        """Filter the given data from a trace

        Use only on uncompressed trace data.

        Args:
            select: A string key to filter.
                    Events containing this string will be included in the filtered result.
        Returns:
            A list of dictionaries containing the filtered data
        """

        if not isinstance(select, str):
            raise RuntimeError("Wrong type for select")

        return [event for event in trace if select in event.values()]

    @staticmethod
    def compress_result(data, key=None):
        """Only use with data containing dicts as follows:

        Simplifies data from
        data = "[{"focus":"1000000"},{"interval":"1000000"},{"interval":"2000000"},{"interval":"2000000"}]

        to

        data = "{"focus": [1000000], "interval": [1000000, 2000000, 2000000"
        """
        result = dict()
        keyset = set([k for e in data for k in e.keys()])
        for k in keyset:
            values = []
            for e in data:
                if k in e:
                    try:
                        values.append(literal_eval(e[k]))
                    except ValueError:
                        values.append(e[k])
            result[k] = values if len(values) > 1 else values[0]
        return result
