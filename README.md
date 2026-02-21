# jura-E6-EB
ESPHome external component to overwrite the firmware on the ESP32-based Jura Wifi Connect dongle with an open version. Works on (at least) the E6 EB model, and probably other Gen7 models as well.
Works with ESPHome 2026.2.1

Here is an example YAML to use (based on the Wifi Connect:

```esphome:
  name: coffee
  friendly_name: Coffee

external_components:
  - source:
      type: local
      path: external_components

esp32:
  board: esp32dev
  framework:
    type: esp-idf

uart:
  tx_pin:
    number: GPIO17
    inverted: false
    mode:
      output: true
      pullup: true
  rx_pin: GPIO16
  baud_rate: 9600
  id: uart_bus

light:
  - platform: status_led
    name: Status
    disabled_by_default: true
    pin: 
      number: GPIO2
      inverted: true

jura_coffee:
  id: my_jura
  uart_id: uart_bus
  single_espresso:
    name: "Single Espressos Count"
    accuracy_decimals: 0
    unit_of_measurement: "cups"
  double_espresso:
    name: "Double Espressos Count"
    accuracy_decimals: 0
    unit_of_measurement: "cups"
  coffee:
    name: "Coffees Made"
    accuracy_decimals: 0
    unit_of_measurement: "cups"
  double_coffee:
    name: "Double Coffees Made"
    accuracy_decimals: 0
    unit_of_measurement: "cups"
  clean:
    name: "Cleaning Count"
    accuracy_decimals: 0
    unit_of_measurement: "cycles"
  tray_status:
    name: "Tray Status"
  bean_status:
    name: "Water Tank Status"
  tank_status:
    name: "Bean Status"
  machine_type:
    name: "Jura E6 EB Type"
  hz_raw:
    name: "HZ Raw"
  coffee_temperature:
    name: "Coffee Heater Temp"
  steam_temperature:
    name: "Steam Heater Temp"
  brew_mode:
    name: "Brew Mode"
  is_brewing:
    name: "Is Brewing"

switch:
  - platform: template
    name: 'Coffee Machine Power'
    icon: "mdi:coffee-maker"
    id: jura_on_off_switch
    turn_on_action:
      - lambda: |-
          id(my_jura).cmd2jura("AN:01");
    turn_off_action:
      - lambda: |-
          id(my_jura).cmd2jura("AN:02");
    optimistic: true
    assumed_state: true

button:
  - platform: template
    name: "Make Espresso"
    icon: "si:coffeescript"
    on_press:
      lambda: |-
        id(my_jura).cmd2jura("FA:04");    
  - platform: template
    name: "Make Coffee"
    icon: "mdi:coffee"
    on_press:
      lambda: |-
        id(my_jura).cmd2jura("FA:05");
  - platform: template
    name: "Make Cappucino"
    icon: "mdi:glass-mug-variant"
    on_press:
      lambda: |-
        id(my_jura).cmd2jura("FA:07");
  - platform: template
    name: "Make Milk Foam"
    icon: "mdi:beer-outline"
    on_press:
      lambda: |-
        id(my_jura).cmd2jura("FA:08");```
