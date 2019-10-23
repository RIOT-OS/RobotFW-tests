# Copyright (c) 2018 Kevin Weiss, for HAW Hamburg  <kevin.weiss@haw-hamburg.de>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.
"""@package PyToAPI
This module handles parsing of information from RIOT periph_uart test.
"""
import logging

from riot_pal import DutShell


class PeriphUartIf(DutShell):
    """Interface to the node with periph_uart firmware."""

    FW_ID = 'periph_uart'
    DEFAULT_DEV = 1
    DEFAULT_BAUD = 115200
    DEFAULT_PARITY = 'N'
    DEFAULT_DATA_BITS = 8
    DEFAULT_STOP_BITS = 1

    def uart_init(self, dev=DEFAULT_DEV, baud=DEFAULT_BAUD):
        """Init UART device"""
        ret = self.send_cmd("uart_init {} {}".format(dev, baud))
        # Clear buffer from init by sending and receiving
        self.send_cmd("uart_write {} {}".format(dev, "flush"))
        return ret

    def uart_mode(self, data_bits=DEFAULT_DATA_BITS, parity=DEFAULT_PARITY,
                  stop_bits=DEFAULT_STOP_BITS, dev=DEFAULT_DEV):
        """Setup databits, parity and stopbits."""
        return self.send_cmd(
            "uart_mode {} {} {} {}".format(dev, data_bits, parity, stop_bits))

    def uart_write(self, data, dev=DEFAULT_DEV):
        """Write data to UART device."""
        return self.send_cmd("uart_write {} {}".format(dev, data))

    def get_metadata(self):
        """Get the metadata of the firmware."""
        return self.send_cmd('get_metadata')

    def get_command_list(self):
        """List of all commands."""
        cmds = list()
        cmds.append(self.get_metadata)
        cmds.append(self.uart_init)
        cmds.append(self.uart_mode)
        cmds.append(self.uart_write)
        return cmds


def main():
    """Test for PeriphUartIf."""

    logging.getLogger().setLevel(logging.DEBUG)
    try:
        uart = PeriphUartIf()
        cmds = uart.get_command_list()
        logging.debug("======================================================")
        for cmd in cmds:
            cmd()
            logging.debug("--------------------------------------------------")
        logging.debug("======================================================")
    except Exception as exc:
        logging.debug(exc)


if __name__ == "__main__":
    main()
