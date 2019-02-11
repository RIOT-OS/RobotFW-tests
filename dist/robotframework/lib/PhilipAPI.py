import serial

from riot_pal import LLMemMapIf, PHILIP_MEM_MAP_PATH
from robot.version import get_version
from time import sleep


class PhilipAPI(LLMemMapIf):

    ROBOT_LIBRARY_SCOPE = 'TEST SUITE'
    ROBOT_LIBRARY_VERSION = get_version()

    def __init__(self, port, baudrate):
        super(PhilipAPI, self).__init__(PHILIP_MEM_MAP_PATH, 'serial', port, baudrate)

    def reset_dut(self):
        ret = list()
        ret.append(self.write_reg('sys.cr', 0xff))
        ret.append(self.execute_changes())
        sleep(1)
        ret.append(self.write_reg('sys.cr', 0x00))
        ret.append(self.execute_changes())
        sleep(1)
        return ret

    def setup_uart(self, mode=0, baudrate=115200,
                   parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
                   rts=True):
        '''Setup tester's UART.'''
        ret = list()
        ret.append(self.write_reg('uart.mode', int(mode)))

        ret.append(self.write_reg('uart.baud', int(baudrate)))

        # setup UART control register
        ctrl = 0
        if parity == serial.PARITY_EVEN:
            ctrl = ctrl | 0x02
        elif parity == serial.PARITY_ODD:
            ctrl = ctrl | 0x04

        if stopbits == serial.STOPBITS_TWO:
            ctrl = ctrl | 0x01
        # invert RTS level as it is a low active signal
        if not rts:
            ctrl = ctrl | 0x08

        ret.append(self.write_reg('uart.ctrl', ctrl))

        # reset status register
        ret.append(self.write_reg('uart.status', 0x00))

        # apply changes
        ret.append(self.execute_changes())
        sleep(1)
        return ret
