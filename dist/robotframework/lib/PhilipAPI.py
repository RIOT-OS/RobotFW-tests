import serial

from philip_pal import Phil
from robot.version import get_version


class PhilipAPI(Phil):
    """Robot framework wrapper for PHiLIP"""
    ROBOT_LIBRARY_SCOPE = 'TEST SUITE'
    ROBOT_LIBRARY_VERSION = get_version()

    def __init__(self, port, baudrate):
        super().__init__(port, baudrate)

    def setup_uart(self, mode=0, baudrate=115200,
                   parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
                   rts=True):
        """Setup tester's UART."""
        ret = list()
        mode = int(mode)
        assert mode >= 0 and mode < 3, "Invalid mode setting for the if_type"
        ret.append(self.write_reg('uart.mode.if_type', mode))
        ret.append(self.write_reg('uart.baud', int(baudrate)))

        # setup UART control register
        if parity == serial.PARITY_EVEN:
            ret.append(self.write_reg('uart.mode.parity', 1))
        elif parity == serial.PARITY_ODD:
            ret.append(self.write_reg('uart.mode.parity', 2))

        if stopbits == serial.STOPBITS_TWO:
            ret.append(self.write_reg('uart.mode.stop_bits', 1))
        # invert RTS level as it is a low active signal
        if not rts:
            ret.append(self.write_reg('uart.mode.rts', 1))

        ret.append(self.write_reg('uart.mode.init', 0))

        # apply changes
        ret.append(self.execute_changes())
        return ret
