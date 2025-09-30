import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import sensor, text_sensor, time

iBoost_ns = cg.esphome_ns.namespace("esphiBoost")
iBoostBuddy = iBoost_ns.class_("iBoostBuddy", cg.PollingComponent)

sx126x_ns = cg.esphome_ns.namespace("sx126x")
SX126x = sx126x_ns.class_("SX126x")

# Optional outputs
CONF_PACKET_COUNT = "packet_count"
CONF_LAST_PACKET = "last_packet"
CONF_HEATING_MODE = "heating_mode"
CONF_HEATING_WARN = "heating_warn"
CONF_HEATING_POWER = "heating_power"
CONF_HEATING_IMPORT = "heating_import"
CONF_HEATING_BOOST = "heating_boost_time"
CONF_HEATING_TODAY = "heating_today"
CONF_HEATING_YESTERDAY = "heating_yesterday"
CONF_HEATING_LAST7 = "heating_last_7"
CONF_HEATING_LAST28 = "heating_last_28"
CONF_HEATING_TOTAL = "heating_last_gt"
CONF_TIME = "time_id"
CONF_RADIO_ID = "radio_id"
CONF_RSSI_IBOOST = "rssi_iboost"
CONF_RSSI_BUDDY = "rssi_buddy"
CONF_RSSI_SENDER = "rssi_sender"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(iBoostBuddy),
            cv.Optional(CONF_PACKET_COUNT): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_LAST_PACKET): cv.use_id(text_sensor.TextSensor),
            cv.Optional(CONF_HEATING_MODE): cv.use_id(text_sensor.TextSensor),
            cv.Optional(CONF_HEATING_WARN): cv.use_id(text_sensor.TextSensor),
            cv.Optional(CONF_HEATING_POWER): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_HEATING_IMPORT): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_HEATING_BOOST): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_HEATING_TODAY): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_HEATING_YESTERDAY): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_HEATING_LAST7): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_HEATING_LAST28): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_HEATING_TOTAL): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_TIME): cv.use_id(time.RealTimeClock),
            cv.Optional(CONF_RADIO_ID): cv.use_id(SX126x),
            cv.Optional(CONF_RSSI_IBOOST): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_RSSI_BUDDY): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_RSSI_SENDER): cv.use_id(sensor.Sensor),
        }
    ).extend(cv.polling_component_schema("10s"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_PACKET_COUNT in config:
        s = await cg.get_variable(config[CONF_PACKET_COUNT])
        cg.add(var.set_packet_count(s))
    if CONF_LAST_PACKET in config:
        t = await cg.get_variable(config[CONF_LAST_PACKET])
        cg.add(var.set_ts_last_packet(t))
    if CONF_HEATING_MODE in config:
        t = await cg.get_variable(config[CONF_HEATING_MODE])
        cg.add(var.set_heating_mode(t))
    if CONF_HEATING_WARN in config:
        t = await cg.get_variable(config[CONF_HEATING_WARN])
        cg.add(var.set_heating_warn(t))
    if CONF_HEATING_POWER in config:
        s = await cg.get_variable(config[CONF_HEATING_POWER])
        cg.add(var.set_heating_power(s))
    if CONF_HEATING_IMPORT in config:
        s = await cg.get_variable(config[CONF_HEATING_IMPORT])
        cg.add(var.set_heating_import(s))
    if CONF_HEATING_BOOST in config:
        s = await cg.get_variable(config[CONF_HEATING_BOOST])
        cg.add(var.set_heating_boost_time(s))
    if CONF_HEATING_TODAY in config:
        s = await cg.get_variable(config[CONF_HEATING_TODAY])
        cg.add(var.set_heating_today(s))
    if CONF_HEATING_YESTERDAY in config:
        s = await cg.get_variable(config[CONF_HEATING_YESTERDAY])
        cg.add(var.set_heating_yesterday(s))
    if CONF_HEATING_LAST7 in config:
        s = await cg.get_variable(config[CONF_HEATING_LAST7])
        cg.add(var.set_heating_last_7(s))
    if CONF_HEATING_LAST28 in config:
        s = await cg.get_variable(config[CONF_HEATING_LAST28])
        cg.add(var.set_heating_last_28(s))
    if CONF_HEATING_TOTAL in config:
        s = await cg.get_variable(config[CONF_HEATING_TOTAL])
        cg.add(var.set_heating_last_gt(s))
    if CONF_TIME in config:
        t = await cg.get_variable(config[CONF_TIME])
        cg.add(var.set_time(t))
    if CONF_RADIO_ID in config:
        r = await cg.get_variable(config[CONF_RADIO_ID])
        cg.add(var.set_radio(r))
    if CONF_RSSI_IBOOST in config:
        s = await cg.get_variable(config[CONF_RSSI_IBOOST])
        cg.add(var.set_rssi_iboost(s))
    if CONF_RSSI_BUDDY in config:
        s = await cg.get_variable(config[CONF_RSSI_BUDDY])
        cg.add(var.set_rssi_buddy(s))
    if CONF_RSSI_SENDER in config:
        s = await cg.get_variable(config[CONF_RSSI_SENDER])
        cg.add(var.set_rssi_sender(s))
