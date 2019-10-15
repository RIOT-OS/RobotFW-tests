from periph_timer_if import PeriphTimerIf
from robot.version import get_version


class PeriphTimer(PeriphTimerIf):

    ROBOT_LIBRARY_SCOPE = 'TEST SUITE'
    ROBOT_LIBRARY_VERSION = get_version()
