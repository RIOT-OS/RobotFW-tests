# Copyright (c) 2018 Kevin Weiss, for HAW Hamburg  <kevin.weiss@haw-hamburg.de>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.
"""@package PyToAPI
This module handles parsing of information from RIOT periph_uart test.
"""
from riot_pal import DutShell


class PeriphUartIf(DutShell):
    """Interface to the node with periph_uart firmware."""
    FW_ID = 'periph_uart'
    DEFAULT_DEV = 1
    DEFAULT_BAUD = 115200

    def uart_init(self, dev=DEFAULT_DEV, baud=DEFAULT_BAUD):
        """Initialize DUT's UART."""
        ret = self.send_cmd("init {} {}".format(dev, baud))
        # Clear buffer from init by sending and receiving
        self.send_cmd("send {} {}".format(dev, "flush"))
        return ret

    def uart_mode(self, data_bits, parity, stop_bits, dev=DEFAULT_DEV,
                  baud=DEFAULT_BAUD):
        """Setup databits, parity and stopbits."""
        self.send_cmd("init {} {}".format(dev, baud))
        ret = self.send_cmd(
            "mode {} {} {} {}".format(dev, data_bits, parity, stop_bits))

        self.send_cmd("send {} {}".format(dev, "flush\n"))
        return ret

    def uart_send_string(self, test_string, dev=DEFAULT_DEV):
        """Send data via DUT's UART."""
        return self.send_cmd("send {} {}".format(dev, test_string))

    def get_metadata(self):
        """Get the metadata of the firmware."""
        return self.send_cmd('get_metadata')


class PeriphUartUtil(object):
    """Helper class for error message handling."""

    @staticmethod
    def uart_get_string(result):
        """Return either an empty string if sending failed or received one."""
        rcv_string = ''
        if result['data'] is not None and result['data'][0]:
            rcv_string = result['data'][0]

        return rcv_string

    def message_for_should_match(self, result, ref_string, stats):
        """Create message for DUT Should Match String keyword."""
        ret = ''
        rcv_string = self.uart_get_string(result)
        if result['result'] == 'Timeout':
            ret = "Test timed out: " + stats
        elif result['result'] == 'Success' and rcv_string != ref_string:
            ret = "Reference string doesn't match the received one: " + stats
        elif result['result'] == 'Error':
            ret = "API call failed: " + result['msg']

        return ret

    def message_for_should_not_match(self, result, ref_string, stats):
        """Create message for DUT Should Not Match String keyword."""
        ret = ''
        rcv_string = self.uart_get_string(result)
        if result['result'] == 'Success' and rcv_string == ref_string:
            ret = "No timeout occurred and the reference string " \
                  "does match the received one: " + stats
        elif result['result'] == 'Error':
            ret = "API call failed: " + result['msg']

        return ret
