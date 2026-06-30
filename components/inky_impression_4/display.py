import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import display, spi
from esphome.const import (
    CONF_BUSY_PIN,
    CONF_DC_PIN,
    CONF_ID,
    CONF_LAMBDA,
    CONF_RESET_PIN,
)

DEPENDENCIES = ["spi"]

inky_impression_4_ns = cg.esphome_ns.namespace("inky_impression_4")
InkyImpression4 = inky_impression_4_ns.class_(
    "InkyImpression4", cg.PollingComponent, display.DisplayBuffer, spi.SPIDevice
)

CONF_BORDER_COLOR = "border_color"
BORDER_COLORS = {
    "BLACK": 0,
    "WHITE": 1,
    "GREEN": 2,
    "BLUE": 3,
    "RED": 4,
    "YELLOW": 5,
    "ORANGE": 6,
    "CLEAN": 7,
}

CONFIG_SCHEMA = (
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(InkyImpression4),
            cv.Required(CONF_DC_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_BUSY_PIN): pins.gpio_input_pin_schema,
            cv.Optional(CONF_BORDER_COLOR, default="WHITE"): cv.enum(
                BORDER_COLORS, upper=True
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(spi.spi_device_schema(cs_pin_required=True))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)
    await spi.register_spi_device(var, config)

    dc_pin = await cg.gpio_pin_expression(config[CONF_DC_PIN])
    cg.add(var.set_dc_pin(dc_pin))

    reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
    cg.add(var.set_reset_pin(reset_pin))

    busy_pin = await cg.gpio_pin_expression(config[CONF_BUSY_PIN])
    cg.add(var.set_busy_pin(busy_pin))

    cg.add(var.set_border_color(config[CONF_BORDER_COLOR]))

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayBufferRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
