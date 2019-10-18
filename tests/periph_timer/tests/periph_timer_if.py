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
    DEFAULT_FREQ = 1000000
    DEFAULT_TICKS = 1000000
    DEFAULT_REPEAT_CNT = 1000000
    DEFAULT_CB_NAME = "cb_toggle"
    DEFAULT_DBG_PORT = 0
    DEFAULT_DBG_PIN = 0

# periph/timer API calls
    def timer_init(self, dev=DEFAULT_TIMER_DEV, freq=DEFAULT_FREQ, cbname=DEFAULT_CB_NAME):
        """Initialize timer device"""
        return self.send_cmd('timer_init {} {} {}'.format(dev, freq, cbname))

    def timer_set(self, dev=DEFAULT_TIMER_DEV, chan=DEFAULT_CHAN, ticks=DEFAULT_TICKS):
        """Set timer in ticks relative."""
        return self.send_cmd('timer_set {} {} {}'.format(dev, chan, ticks))

    def timer_set_absolute(self, dev=DEFAULT_TIMER_DEV, chan=DEFAULT_CHAN, ticks=DEFAULT_TICKS):
        """Set timer in ticks absolute."""
        return self.send_cmd('timer_set {} {} {}'.format(dev, chan, ticks))

    def timer_clear(self, dev=DEFAULT_TIMER_DEV, chan=DEFAULT_CHAN):
        """Clear timer channel."""
        return self.send_cmd('timer_clear {} {}'.format(dev, chan))

    def timer_read(self, dev=DEFAULT_TIMER_DEV):
        """Read timer ticks."""
        return self.send_cmd('timer_read {}'.format(dev))

    def timer_start(self, dev=DEFAULT_TIMER_DEV):
        """Start timer."""
        return self.send_cmd('timer_start {}'.format(dev))

    def timer_stop(self, dev=DEFAULT_TIMER_DEV):
        """Stop timer."""
        return self.send_cmd('timer_stop {}'.format(dev))

# additional helper calls, to allow for advanced timing tests
    def timer_debug_pin(self, dev=DEFAULT_TIMER_DEV,
                        port=DEFAULT_DBG_PORT, pin=DEFAULT_DBG_PIN):
        """Configure debug pin for timer device."""
        return self.send_cmd('timer_debug_pin {} {} {}'.format(dev, port, pin))

    def timer_read_bench(self, dev=DEFAULT_TIMER_DEV,
                         repeat_cnt=DEFAULT_REPEAT_CNT):
        """Bench timer read time overhead."""
        return self.send_cmd('timer_read_bench {} {}'.format(dev, repeat_cnt))

# util calls
    def get_metadata(self):
        """Get the metadata of the firmware."""
        return self.send_cmd('get_metadata')

    def get_command_list(self):
        """List of all commands."""
        cmds = list()
        cmds.append(self.get_metadata)
        cmds.append(self.timer_init)
        cmds.append(self.timer_set)
        cmds.append(self.timer_set_absolute)
        cmds.append(self.timer_clear)
        cmds.append(self.timer_read)
        cmds.append(self.timer_start)
        cmds.append(self.timer_stop)
        cmds.append(self.timer_debug_pin)
        cmds.append(self.timer_read_bench)
        return cmds


def main():
    """Test for PeriphTimerIf."""

    logging.getLogger().setLevel(logging.DEBUG)
    try:
        ptimer = PeriphTimerIf()
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
