# Copyright (C) 2019 Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.
"""@package PyToAPI
This module handles parsing of information from RIOT periph_timer_cli test.
"""
import logging

from riot_pal import DutShell


class PeriphTimerIf(DutShell):
    """Interface to the a node with periph_timer_cli firmware."""


    FW_ID = 'periph_timer_cli'
    DEFAULT_TIMER_DEV = 0
    DEFAULT_CHAN = 0
    DEFAULT_FREQ =  1000000
    DEFAULT_TICKS = 1000000
    DEFAULT_REPEAT_CNT = 1000000
    DEFAULT_CB_NAME = "cb_toggle"
    DEFAULT_DBG_PORT = 0
    DEFAULT_DBG_PIN = 0

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def is_connected_to_board(self):
        """Checks if board is connected."""
        return self.i2c_get_id()["data"] == [self.FW_ID]

    def timer_init(self, dev=DEFAULT_TIMER_DEV, freq=DEFAULT_FREQ,
                   cbname=DEFAULT_CB_NAME, gpio_port=DEFAULT_DBG_PORT,
                   gpio_pin=DEFAULT_DBG_PIN):
        """Get current timer ticks."""
        return self.send_cmd('timer_init {} {} {} {} {}'.format(dev, freq,
                                                                cbname,
                                                                gpio_port,
                                                                gpio_pin))

    def timer_set(self, dev=DEFAULT_TIMER_DEV, chan=DEFAULT_CHAN,
                  ticks=DEFAULT_TICKS, gpio_port=DEFAULT_DBG_PORT,
                  gpio_pin=DEFAULT_DBG_PIN):
        """Set timer in ticks."""
        return self.send_cmd('timer_set {} {} {} {} {}'.format(dev, chan, ticks,
                                                               gpio_port,
                                                               gpio_pin))

    def timer_read_bench(self, dev=DEFAULT_TIMER_DEV,
                         repeat_cnt=DEFAULT_REPEAT_CNT,
                         gpio_port=DEFAULT_DBG_PORT,
                         gpio_pin=DEFAULT_DBG_PIN):
        """Bench timer read time overhead."""
        return self.send_cmd('timer_read_bench {} {} {} {}'.format(dev,
                                                                   repeat_cnt,
                                                                   gpio_port,
                                                                   gpio_pin))


    def get_metadata(self):
        """Get the metadata of the firmware."""
        return self.send_cmd('get_metadata')

    def get_command_list(self):
        """List of all commands."""
        cmds = list()
        cmds.append(self.get_metadata)
        cmds.append(self.timer_init)
        cmds.append(self.timer_set)
        return cmds


def main():
    """Test for PeriphTimer."""

    logging.getLogger().setLevel(logging.DEBUG)
    try:
        ptimer = PeriphTimer()
        cmds = ptimer.get_command_list()
        logging.debug("======================================================")
        for cmd in cmds:
            cmd()
            logging.debug("--------------------------------------------------")
        logging.debug("======================================================")
    except Exception as exc:
        logging.debug(exc)


if __name__ == "__main__":
    main()
