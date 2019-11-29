*** Settings ***
Documentation       Verify if board to philip wiring is correct.

Suite Setup         Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Firmware Should Match
Test Setup          Run Keywords    PHILIP Reset
...                                 RIOT Reset
...                                 API Sync Shell

Test Template       Verify GPIO Pin Is Connected

Resource            periph_gpio.keywords.txt

Force Tags          periph  gpio

*** Test Cases ***  PHILIP_GPIO             DUT_PORT        DUT_PIN
Verify GPIO_0       gpio[0].status.level    %{HIL_DUT_GPIO0_PORT}  %{HIL_DUT_GPIO0_PIN}
Verify GPIO_1       gpio[1].status.level    %{HIL_DUT_GPIO1_PORT}  %{HIL_DUT_GPIO1_PIN}
Verify GPIO_2       gpio[2].status.level    %{HIL_DUT_GPIO2_PORT}  %{HIL_DUT_GPIO2_PIN}
