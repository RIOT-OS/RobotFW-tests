# Copyright (C) 2019 Kevin Weiss <kevin.weiss@haw-hamburg.de>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.
"""@package PyToAPI
This module handles parsing of information from RIOT periph_spi test.
"""
import logging

from riot_pal import DutShell


class PeriphSpiIf(DutShell):
    """Interface to the a node with periph_spi firmware."""

    def spi_init(self, dev):
        """Basic initialization of the given SPI bus"""
        return self.send_cmd('spi_init {}'.format(dev))

    def spi_init_pins(self, dev):
        """Initialize the used SPI bus pins, i.e. MISO, MOSI, and CLK"""
        return self.send_cmd('spi_init_pins {}'.format(dev))

    def spi_init_cs(self, dev, port, pin):
        """Initialize the given chip select pin"""
        return self.send_cmd('spi_init_cs {} {} {}'.format(dev, port, pin))

    def spi_init_with_gpio_mode(self, dev):
        """Initialize MOSI/MISO/SCLK pins with adapted GPIO modes"""
        return self.send_cmd('spi_init_with_gpio_mode {}'.format(dev))

    def spi_acquire(self, dev, port, pin, mode, clk):
        """Start a new SPI transaction"""
        return self.send_cmd('spi_acquire {} {} {} {} {}'.format(dev, mode, clk, port, pin))

    def spi_release(self, dev):
        """Finish an ongoing SPI transaction by releasing the given SPI bus"""
        return self.send_cmd('spi_release {}'.format(dev))

    def spi_transfer_byte(self, dev, port, pin, cont, out):
        """Transfer one byte on the given SPI bus"""
        return self.send_cmd('spi_transfer_byte {} {} {} {} {}'.format(dev, port, pin, cont, out))

    def spi_transfer_bytes(self, dev, port, pin, cont, in_len, out=None):
        """Transfer a number bytes using the given SPI bus"""
        if out:
            return self.send_cmd('spi_transfer_bytes {} {} {} {} {} {}'.format(dev, port, pin, cont, in_len, out))
        else:
            return self.send_cmd('spi_transfer_bytes {} {} {} {} {}'.format(dev, port, pin, cont, in_len))

    def spi_transfer_reg(self, dev, port, pin, reg, out):
        """Transfer one byte to/from a given register address"""
        return self.send_cmd('spi_transfer_reg {} {} {} {} {}'.format(dev, port, pin, reg, out))

    def spi_transfer_regs(self, dev, port, pin, reg, in_len, out=None):
        """Transfer a number bytes using the given SPI bus"""
        if out:
            return self.send_cmd('spi_transfer_regs {} {} {} {} {} {}'.format(dev, port, pin, reg, in_len, out))
        else:
            return self.send_cmd('spi_transfer_regs {} {} {} {} {}'.format(dev, port, pin, reg, in_len))

    def spi_get_devs(self):
        """Gets amount of supported spi devices."""
        return self.send_cmd('spi_get_devs')

    def get_metadata(self):
        """Get the metadata of the firmware."""
        res = self.send_cmd('get_metadata')
        res['msg'] = res['data'][1] 
        return res
