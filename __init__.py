import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, text_sensor, binary_sensor
from esphome.const import CONF_ID, CONF_UART_ID, CONF_UPDATE_INTERVAL

jura_coffee_ns = cg.esphome_ns.namespace("jura_coffee")
JuraCoffee = jura_coffee_ns.class_("JuraCoffee", cg.PollingComponent, uart.UARTDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.declare_id(JuraCoffee),
        cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        cv.Optional("single_espresso"): sensor.sensor_schema(
            unit_of_measurement="",
            icon="mdi:coffee",
            accuracy_decimals=0,
            state_class="total_increasing",
        ),
        cv.Optional("double_espresso"): sensor.sensor_schema(
            unit_of_measurement="",
            icon="mdi:coffee",
            accuracy_decimals=0,
            state_class="total_increasing",
        ),
        cv.Optional("coffee"): sensor.sensor_schema(
            unit_of_measurement="",
            icon="mdi:coffee",
            accuracy_decimals=0,
            state_class="total_increasing",
        ),
        cv.Optional("double_coffee"): sensor.sensor_schema(
            unit_of_measurement="",
            icon="mdi:coffee",
            accuracy_decimals=0,
            state_class="total_increasing",
        ),
        cv.Optional("clean"): sensor.sensor_schema(
            unit_of_measurement="",
            icon="mdi:coffee-maker",
            accuracy_decimals=0,
            state_class="total_increasing",
        ),
        cv.Optional("tray_status"): text_sensor.text_sensor_schema(icon="mdi:tray"),
        cv.Optional("bean_status"): text_sensor.text_sensor_schema(icon="mdi:coffee-bean"),
        cv.Optional("tank_status"): text_sensor.text_sensor_schema(icon="mdi:water"),
        cv.Optional("machine_type"): text_sensor.text_sensor_schema(
            icon="mdi:information-outline",
            entity_category="diagnostic",
        ),
        cv.Optional("hz_raw"): text_sensor.text_sensor_schema(
            icon="mdi:information-outline", entity_category="diagnostic"
        ),
        cv.Optional("coffee_temperature"): sensor.sensor_schema(
            unit_of_measurement="°C",
            icon="mdi:thermometer",
            accuracy_decimals=1,
            state_class="measurement",
        ),
        cv.Optional("steam_temperature"): sensor.sensor_schema(
            unit_of_measurement="°C",
            icon="mdi:thermometer",
            accuracy_decimals=1,
            state_class="measurement",
        ),
        cv.Optional("brew_mode"): text_sensor.text_sensor_schema(icon="mdi:coffee-maker"),
        cv.Optional("is_brewing"): binary_sensor.binary_sensor_schema(icon="mdi:coffee"),
    }).extend(cv.polling_component_schema("10s")),
)

async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        await cg.get_variable(config[CONF_UART_ID])
    )
    await cg.register_component(var, config)

    for key, setter in (
        ("single_espresso", "set_single_espresso_sensor"),
        ("double_espresso", "set_double_espresso_sensor"),
        ("coffee", "set_coffee_sensor"),
        ("double_coffee", "set_double_coffee_sensor"),
        ("clean", "set_clean_sensor"),
        ("coffee_temperature", "set_coffee_temperature_sensor"),
        ("steam_temperature", "set_steam_temperature_sensor"),
    ):
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, setter)(sens))

    for key, setter in (
        ("tray_status", "set_tray_status_sensor"),
        ("bean_status", "set_bean_status_sensor"),
        ("tank_status", "set_tank_status_sensor"),
        ("machine_type", "set_machine_type_sensor"),
        ("hz_raw", "set_hz_raw_sensor"),
        ("brew_mode", "set_brew_mode_sensor"),
    ):
        if key in config:
            ts = await text_sensor.new_text_sensor(config[key])
            cg.add(getattr(var, setter)(ts))

    for key, setter in (
        ("is_brewing", "set_is_brewing_sensor"),
    ):
        if key in config:
            bs = await binary_sensor.new_binary_sensor(config[key])
            cg.add(getattr(var, setter)(bs))
