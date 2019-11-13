include ../Makefile.tests_common

export RF_UART_DEV ?= 1

FEATURES_REQUIRED += periph_uart
FEATURES_OPTIONAL += periph_lpuart  # STM32 L0 and L4 provides lpuart support
FEATURES_OPTIONAL += periph_uart_modecfg

USEMODULE += shell
USEMODULE += xtimer

include $(RIOTBASE)/Makefile.include
