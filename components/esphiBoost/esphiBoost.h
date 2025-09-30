#pragma once
#include "esphome.h"
#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/time/real_time_clock.h"
#include <vector>

namespace esphome {
    namespace sx126x { class SX126x; }
}

namespace esphome {
    namespace esphiBoost {

        enum ControlPacketAction { 
            CONTROL_PACKET_ACTION_REQUEST_DATA = 0,
            CONTROL_PACKET_ACTION_BOOST_START = 1,
            CONTROL_PACKET_ACTION_BOOST_CANCEL = 2,
        };

        class iBoostBuddy : public PollingComponent {
        public:

            iBoostBuddy() : PollingComponent(10000) {} // poll every 10 seconds (fixed from 100000)

            void set_time(time::RealTimeClock * t) { rtc_ = t; }
            void set_radio(esphome::sx126x::SX126x *r) { radio_ = r; }

            // Optional wiring for sensors/text_sensors
            void set_packet_count(sensor::Sensor *s) { packet_count_ = s; }
            void set_ts_last_packet(text_sensor::TextSensor *s) { ts_last_packet_ = s; }
            void set_heating_mode(text_sensor::TextSensor *s) { heating_mode_ = s; }
            void set_heating_warn(text_sensor::TextSensor *s) { heating_warn_ = s; }
            void set_heating_power(sensor::Sensor *s) { heating_power_ = s; }
            void set_heating_import(sensor::Sensor *s) { heating_import_ = s; }
            void set_heating_boost_time(sensor::Sensor *s) { heating_boost_time_ = s; }
            void set_heating_today(sensor::Sensor *s) { heating_today_ = s; }
            void set_heating_yesterday(sensor::Sensor *s) { heating_yesterday_ = s; }
            void set_heating_last_7(sensor::Sensor *s) { heating_last_7_ = s; }
            void set_heating_last_28(sensor::Sensor *s) { heating_last_28_ = s; }
            void set_heating_last_gt(sensor::Sensor *s) { heating_last_gt_ = s; }
            void set_rssi_iboost(sensor::Sensor *s) { rssi_iboost_ = s; }
            void set_rssi_buddy(sensor::Sensor *s) { rssi_buddy_ = s; }
            void set_rssi_sender(sensor::Sensor *s) { rssi_sender_ = s; }

            // Operations
            void boost_start(uint8_t minutes);
            void boost_cancel();

            // ESPHome lifecycle
            void setup() override;
            void loop() override;   // Receiving done via callback -> process_packet()
            void update() override; // Polling trigger (send ping cycle)
            void dump_config() override;

            // Radio I/O exposed for the sx126x callback and send helpers
            void process_packet(const std::vector<uint8_t> &x, float rssi); // called by the sx126x

        private:
            // Packet processing helpers

            // Packet handlers
            void handle_packet_iboost_(const std::vector<uint8_t> &buffer, float rssi);
            void handle_packet_buddy_(const std::vector<uint8_t> &buffer, float rssi);
            void handle_packet_sender_(const std::vector<uint8_t> &buffer, float rssi);

            // Internal helpers
            void send_packet_(const std::vector<uint8_t> &packet);
            void update_and_publish_packet_count_();
            void send_control_packet_(ControlPacketAction packet_mode, uint8_t boost_minutes);
            uint8_t get_next_data_request_code_();

            // Sensors
            sensor::Sensor *packet_count_ = nullptr;
            text_sensor::TextSensor *ts_last_packet_ = nullptr;
            text_sensor::TextSensor *heating_mode_ = nullptr;
            text_sensor::TextSensor *heating_warn_ = nullptr;
            sensor::Sensor *heating_power_ = nullptr;
            sensor::Sensor *heating_import_ = nullptr;
            sensor::Sensor *heating_boost_time_ = nullptr;
            sensor::Sensor *heating_today_ = nullptr;
            sensor::Sensor *heating_yesterday_ = nullptr;
            sensor::Sensor *heating_last_7_ = nullptr;
            sensor::Sensor *heating_last_28_ = nullptr;
            sensor::Sensor *heating_last_gt_ = nullptr;
            sensor::Sensor *rssi_iboost_ = nullptr;   // Last RSSI seen from iBoost main unit
            sensor::Sensor *rssi_buddy_ = nullptr;    // Last RSSI seen from Buddy unit
            sensor::Sensor *rssi_sender_ = nullptr;   // Last RSSI seen from Sender unit

            time::RealTimeClock *rtc_ = nullptr;
            esphome::sx126x::SX126x *radio_ = nullptr; // native driver
        };

    } // namespace esphiBoost
} // namespace esphome
