def str_4():
    result = ""
    for i in range(3, 16 + 3):
        result += str(i) + " "
    return result

LIST__VAL_1 = [254]
LIST__VAL_2 = [41]
LIST__VAL_3 = [41, 42]

VAL_4 = str_4()
LIST__VAL_5 = [254] + list(range(3, 16 + 3 - 1))

LIST__VAL_6 = [5]
LIST__VAL_7 = [42, 43]
LIST__VAL_8 = [5, 6, 7] + list(range(44, 16 + 44 - 3))

LIST__VAL_9 = [254, 7, 8, 9, 10]
LIST__VAL_10 = [254, 11, 2, 14, 5]


WARN_TOLERANCE = 0.1 #in percentage
FAIL_TOLERANCE = 0.5 #in percentage
SEC_TO_USEC = 1000000
BITS_PER_BYTE = 8

def spi_speed_comparison(expected_freq, frame_stats, sys_clock_speed, size):
    expected_freq = int(expected_freq)
    sys_clock_speed = int(sys_clock_speed)
    size = int(size)

    measured_freq = frame_stats['mean']
    measured_byte_count =  len(frame_stats['values'])
    assert measured_byte_count == size, 'Expected byte count does not match measured byte count'

    result = {}
    result['pass'] = False
    result['warn'] = False
    result['measured_freq'] = measured_freq
    result['difference_percentage'] = 0
    result['byte_count'] = measured_byte_count

    warn_limits = calculate_limits(expected_freq, WARN_TOLERANCE)
    fail_limits = calculate_limits(expected_freq, FAIL_TOLERANCE)

    if measured_freq >= fail_limits['lower_limit'] and measured_freq <= fail_limits['upper_limit']:
        result['pass'] = True
        if measured_freq < warn_limits['lower_limit'] or measured_freq > warn_limits['upper_limit']:
            result['warn'] = True

    result['difference_percentage'] = round((measured_freq - expected_freq) / expected_freq * 100, 2)

    return result

def calculate_limits(expected_freq, tolerance):
    result = {}
    result['lower_limit'] = int(SEC_TO_USEC / (1 / (expected_freq * (1 - tolerance)) * SEC_TO_USEC))
    result['upper_limit'] = int(expected_freq * (1 + tolerance))
    return result

def ticks_to_us(ticks, sys_clock_speed):
    sToUs = 1000000
    return round(ticks / sys_clock_speed * sToUs, 3)
