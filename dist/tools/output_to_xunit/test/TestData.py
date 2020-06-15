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

import random


class TestData():
    """Interface to the node."""

    ROBOT_LIBRARY_SCOPE = "TEST"
    ROBOT_LIBRARY_VERSION = get_version()

    def get_empty(self):
        return ""

    def get_long_list(self):
        return [i for i in range(0, 500)]

    def get_long_dict(self):
        return {f"{str(i)}": i**2 for i in range(0, 500)}

    def get_long_string(self):
        return ''.join(random.choices("HEYDSLKJSDFLKJD", k=1000))
