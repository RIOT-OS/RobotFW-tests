from periph_gpio_if import PeriphGpioIf
from robot.version import get_version


class GPIOdevice(PeriphGpioIf):

    ROBOT_LIBRARY_SCOPE = 'TEST SUITE'
    ROBOT_LIBRARY_VERSION = get_version()
