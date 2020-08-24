# Copyright (C) 2019 Kevin Weiss <kevin.weiss@haw-hamburg.de>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.
"""@package PyToAPI
This module handles parsing of information from RIOT periph_gpio test.
"""
from riot_pal import DutShell


class PeriphGpioIf(DutShell):
    """Interface to the a node with periph_i2c firmware."""

    FW_ID = 'periph_gpio'

    @staticmethod
    def _convert_port_to_num(port):
        port = str(port).upper()
        port = port.strip("PORT_")
        port = port.strip("PORT")
        port = port.strip("P_")
        port = port.strip("P")
        if ord(port[0]) >= ord('A') and ord(port[0]) <= ord('Z'):
            port = ord(port[0]) - ord('A')
        return int(port)

    def gpio_set(self, port, pin):
        """Set the GPIO port and pin to HIGH."""

        return self.send_cmd('gpio_set {} {}'
                             .format(self._convert_port_to_num(port), pin))

    def gpio_clear(self, port, pin):
        """Clear the GPIO port and pin to LOW."""

        return self.send_cmd('gpio_clear {} {}'
                             .format(self._convert_port_to_num(port), pin))

    def get_metadata(self):
        """Get the metadata of the firmware."""
        return self.send_cmd('get_metadata')

    def lock(self):
        """Lock the device"""
        return self.send_cmd('lock')

    def get_command_list(self):
        """List of all commands."""
        cmds = list()
        cmds.append(self.gpio_set)
        cmds.append(self.gpio_clear)
        cmds.append(self.get_metadata)
        return cmds
