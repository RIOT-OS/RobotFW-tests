include ../Makefile.tests_common

FEATURES_REQUIRED = periph_gpio

USEMODULE += shell
USEMODULE += random
USEMODULE += xtimer

USE_JSON_SHELL_PARSER ?= 1

ifeq (,$(HIL_DUT_IC_PIN))
$(error	HIL_DUT_IC_PIN not defined)
endif
ifeq (,$(HIL_DUT_IC_PORT))
$(error	HIL_DUT_IC_PORT not defined)
endif

CFLAGS+= -DTIMER_TEST_PIN=$(HIL_DUT_IC_PIN) -DTIMER_TEST_PORT=$(HIL_DUT_IC_PORT)


include $(RIOTBASE)/Makefile.include
