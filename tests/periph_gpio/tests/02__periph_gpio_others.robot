*** Settings ***
Documentation       Verify if board to philip wiring is correct.

Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Firmware Should Match
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell

Test Template       Verify Periph Pin Is Connected


Resource            periph_gpio.keywords.txt

Force Tags          periph  gpio


*** Test Cases ***  PHILIP_PERIPH   PHILIP_GPIO     DUT_PORT                DUT_PIN

Verify UART_CTS     uart            dut_cts         %{HIL_DUT_CTS_PORT}     %{HIL_DUT_CTS_PIN}
Verify UART_RTS     uart            dut_rts         %{HIL_DUT_RTS_PORT}     %{HIL_DUT_RTS_PIN}
Verify UART_RX      uart            dut_rx          %{HIL_DUT_RX_PORT}      %{HIL_DUT_RX_PIN}
Verify UART_TX      uart            dut_tx          %{HIL_DUT_TX_PORT}      %{HIL_DUT_TX_PIN}
Verify SPI_NSS      spi             dut_nss         %{HIL_DUT_NSS_PORT}     %{HIL_DUT_NSS_PIN}
Verify SPI_SCK      spi             dut_sck         %{HIL_DUT_SCK_PORT}     %{HIL_DUT_SCK_PIN}
Verify SPI_MISO     spi             dut_miso        %{HIL_DUT_MISO_PORT}    %{HIL_DUT_MISO_PIN}
Verify SPI_MOSI     spi             dut_mosi        %{HIL_DUT_MOSI_PORT}    %{HIL_DUT_MOSI_PIN}
Verify I2C_SCL      i2c             dut_scl         %{HIL_DUT_SCL_PORT}     %{HIL_DUT_SCL_PIN}
Verify I2C_SDA      i2c             dut_sda         %{HIL_DUT_SDA_PORT}     %{HIL_DUT_SDA_PIN}
Verify ADC          adc             dut_adc         %{HIL_DUT_ADC_PORT}     %{HIL_DUT_ADC_PIN}
Verify PWM          pwm             dut_pwm         %{HIL_DUT_PWM_PORT}     %{HIL_DUT_PWM_PIN}
Verify IC           tmr             dut_ic          %{HIL_DUT_IC_PORT}      %{HIL_DUT_IC_PIN}
