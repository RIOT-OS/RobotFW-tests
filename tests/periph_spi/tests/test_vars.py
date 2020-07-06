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


def spi_speed_limits(spi_speed, sys_clock_speed, bytes):

	for i in range(2):
		if i == 0:
			spi_speed_limit = spi_speed * 1.20
		else:
			spi_speed_limit = spi_speed * 0.8

		expected_byte_ticks = round((8 * sys_clock_speed) / spi_speed_limit)
		expected_frame_ticks = expected_byte_ticks * bytes
		error_tol = round(expected_byte_ticks * bytes / 8)

		if i == 0:
			lower_limit = expected_frame_ticks - error_tol
		else:
			upper_limit = expected_frame_ticks + error_tol

	return [lower_limit, upper_limit]


def convert_ticks_to_us(ticks, sys_clock_speed):
	return round(ticks / sys_clock_speed * 1000000, 3)
