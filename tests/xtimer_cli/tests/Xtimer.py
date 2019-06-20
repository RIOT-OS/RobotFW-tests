from xtimer_if import XtimerIf
from robot.version import get_version


class Xtimer(XtimerIf):

    ROBOT_LIBRARY_SCOPE = 'TEST SUITE'
    ROBOT_LIBRARY_VERSION = get_version()
