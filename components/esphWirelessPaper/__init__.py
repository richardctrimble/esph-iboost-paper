import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

print("WirelessPaper Display component is being loaded!")

esphWirelessPaper_ns = cg.esphome_ns.namespace("esphWirelessPaper")
PaperDisplay = esphWirelessPaper_ns.class_("PaperDisplay", cg.PollingComponent)

CONFIG_SCHEMA = (
    cv.Schema(
      {
        cv.GenerateID(): cv.declare_id(PaperDisplay),
        cv.Optional("top_title", default=""):  cv.string,
      }
    ).extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Set configuration values
    if "top_title" in config:
        cg.add(var.set_TopTitle(config["top_title"]))
