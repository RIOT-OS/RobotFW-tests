include ../Makefile.tests_common

FEATURES_REQUIRED += periph_uart
FEATURES_OPTIONAL += periph_lpuart  # STM32 L0 and L4 provides lpuart support
FEATURES_OPTIONAL += periph_uart_modecfg
FEATURES_REQUIRED += HIL_UART_DEV

USEMODULE += shell
USEMODULE += xtimer

export HIL_UART_DEV

include $(RIOTBASE)/Makefile.include
