from periph_spi_if import PeriphSpiIf
from robot.version import get_version


class SPIdevice(PeriphSpiIf):

    ROBOT_LIBRARY_SCOPE = 'TEST SUITE'
    ROBOT_LIBRARY_VERSION = get_version()
