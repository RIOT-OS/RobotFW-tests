import errno
import random
import string


def increment_data(data):
    """Increment each character."""
    return ''.join(chr(ord(n) + 1) for n in data)


def create_random_data(data_len):
    """Generate random data with specified length."""
    return 't' + ''.join([random.choice(
        string.digits) for n in range(data_len)])


SHORT_TEST_STRING = "t111"
SHORT_TEST_STRING_INC = increment_data(SHORT_TEST_STRING)
LONG_TEST_STRING = create_random_data(42)
LONG_TEST_STRING_INC = increment_data(LONG_TEST_STRING)

REG_USER_READ = "\"rr 0 10\""
REG_USER_READ_DATA = ['{"data":[0,1,2,3,4,5,6,7,8,9']
REG_WRONG_READ = "\"rr -1 10\""
REG_WRONG_READ_DATA = ['{"result":22}']
