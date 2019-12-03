include ../Makefile.tests_common

FEATURES_REQUIRED = periph_spi

USEMODULE += shell
USE_JSON_SHELL_PARSER ?= 1

export HIL_SPI_DEV
export HIL_DUT_NSS_PORT
export HIL_DUT_NSS_PIN

include $(RIOTBASE)/Makefile.include
