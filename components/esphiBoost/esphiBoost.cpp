#include "esphiBoost.h"
#include "esphome/core/log.h"

namespace esphome
{
    namespace esphiBoost
    {
        static constexpr const char *TAG_IBOOST = "esphiBoost";

        // Request and response codes for iBoost data queries
        enum DataRequestCodes
        {
            DATA_REQUEST_TODAY = 0xCA,        // Request today's energy savings
            DATA_REQUEST_YESTERDAY = 0xCB,    // Request yesterday's energy savings
            DATA_REQUEST_LAST_7_DAYS = 0xCC,  // Request last 7 days energy savings
            DATA_REQUEST_LAST_28_DAYS = 0xCD, // Request last 28 days energy savings
            DATA_REQUEST_TOTAL = 0xCE         // Request total energy savings
        };

        // Packet type identifiers for different device types in the iBoost system
        enum PacketTypes
        {
            PACKET_TYPE_SENDER = 0x01, // Sender unit packet (value: 1)
            PACKET_TYPE_BUDDY = 0x21,  // Buddy unit packet (value: 33) - indicates buddy is running
            PACKET_TYPE_IBOOST = 0x22  // iBoost main unit packet (value: 34)
        };

        // Global state variables for iBoost system communication
        bool is_system_address_valid_ = false;
        int system_address_byte0_ = -1;
        int system_address_byte1_ = -1;
        float system_address_rssi_ = -1000.0f;
        bool is_sender_battery_low_ = false;
        bool iboost_unit_overheated = false;

        // Data request cycle for polling different energy statistics
        static const uint8_t DATA_REQUEST_CYCLE[] = {
            DATA_REQUEST_TODAY,
            DATA_REQUEST_YESTERDAY,
            DATA_REQUEST_LAST_7_DAYS,
            DATA_REQUEST_LAST_28_DAYS,
            DATA_REQUEST_TOTAL};
        static size_t data_request_cycle_index_ = 0; // Current index in the data request cycle

        // private functions
        void iBoostBuddy::send_packet_(const std::vector<uint8_t> &packet)
        {
            if (!radio_)
            {
                ESP_LOGE(TAG_IBOOST, "SX126x radio not configured; cannot transmit packet");
                return;
            }

            ESP_LOGVV(TAG_IBOOST, "TX: Transmitting packet data: %s", format_hex_pretty(packet).c_str());
            auto transmission_result = radio_->transmit_packet(packet);
            ESP_LOGVV(TAG_IBOOST, "TX: Transmission result code: %d", static_cast<int>(transmission_result));
        }

        void iBoostBuddy::update_and_publish_packet_count_()
        {
            // Use local static to maintain packet count without class-wide mutable state
            static uint32_t total_packet_count = 0;

            // Increment and publish packet count if sensor is configured
            if (packet_count_)
            {
                packet_count_->publish_state(++total_packet_count);
            }

            // Update timestamp of last received packet if both RTC and timestamp sensor are available
            if (rtc_ && ts_last_packet_)
            {
                auto current_time = rtc_->now();
                if (current_time.is_valid())
                {
                    char timestamp_buffer[32];
                    snprintf(timestamp_buffer, sizeof(timestamp_buffer),
                             "%04d-%02d-%02dT%02d:%02d:%02dZ",
                             current_time.year, current_time.month, current_time.day_of_month,
                             current_time.hour, current_time.minute, current_time.second);
                    ts_last_packet_->publish_state(timestamp_buffer);
                }
            }
        }

        uint8_t iBoostBuddy::get_next_data_request_code_()
        {
            // Get the next data request code from the cycle (0xCA through 0xCE)
            uint8_t current_request_code = DATA_REQUEST_CYCLE[data_request_cycle_index_];

            // Advance to next position in cycle, wrapping around at end
            data_request_cycle_index_ = (data_request_cycle_index_ + 1) %
                                        (sizeof(DATA_REQUEST_CYCLE) / sizeof(DATA_REQUEST_CYCLE[0]));

            return current_request_code;
        }

        void iBoostBuddy::send_control_packet_(ControlPacketAction packet_mode, uint8_t boost_minutes)
        {
            if (!is_system_address_valid_)
            {
                ESP_LOGD(TAG_IBOOST, "TX: Cannot send control message - waiting for system address discovery...");
                return;
            }

            // Determine descriptive message for logging based on packet mode
            std::string mode_description = "Request Data";
            if (packet_mode == CONTROL_PACKET_ACTION_BOOST_START)
            {
                mode_description = "Start Boost";
            }
            else if (packet_mode == CONTROL_PACKET_ACTION_BOOST_CANCEL)
            {
                mode_description = "Cancel Boost";
            }

            std::vector<uint8_t> control_packet(29, 0);
            control_packet[0] = static_cast<uint8_t>(system_address_byte0_);
            control_packet[1] = static_cast<uint8_t>(system_address_byte1_);
            control_packet[2] = PACKET_TYPE_BUDDY; // Buddy packet type

            if (packet_mode == CONTROL_PACKET_ACTION_REQUEST_DATA)
            {
                control_packet[3] = 0x08; // Request group data command
            }
            else
            {
                control_packet[3] = 0x18; // Set boost time command
            }

            // Set fixed protocol bytes
            control_packet[4] = 0x92;
            control_packet[5] = 0x07;
            control_packet[8] = 0x24;
            control_packet[10] = 0xA0;
            control_packet[11] = 0xA0;
            control_packet[12] = get_next_data_request_code_(); // Cycle through data request codes
            control_packet[14] = 0xA0;
            control_packet[15] = 0xA0;
            control_packet[16] = 0xC8;
            std::string request_display_name = "";
            if (packet_mode != CONTROL_PACKET_ACTION_REQUEST_DATA)
            {
                if (packet_mode == CONTROL_PACKET_ACTION_BOOST_CANCEL)
                {
                    control_packet[17] = 0; // Cancel boost - set minutes to 0
                }
                else
                {
                    control_packet[17] = boost_minutes; // Set boost duration in minutes
                }
            }
            else
            {
                // Log the specific data request being sent
                switch (control_packet[12])
                {
                case DATA_REQUEST_TODAY:
                    request_display_name = "Saved Today";
                    break;
                case DATA_REQUEST_YESTERDAY:
                    request_display_name = "Saved Yesterday";
                    break;
                case DATA_REQUEST_LAST_7_DAYS:
                    request_display_name = "SavedLast 7 Days";
                    break;
                case DATA_REQUEST_LAST_28_DAYS:
                    request_display_name = "SavedLast 28 Days";
                    break;
                case DATA_REQUEST_TOTAL:
                    request_display_name = "Saved Total";
                    break;
                }
            }
            send_packet_(control_packet);
            ESP_LOGD(TAG_IBOOST, "TX: Sent control packet [%s][%s]", mode_description.c_str(), request_display_name.c_str());
        }

        void iBoostBuddy::boost_start(uint8_t minutes)
        {
            send_control_packet_(CONTROL_PACKET_ACTION_BOOST_START, minutes);
        }

        void iBoostBuddy::boost_cancel()
        {
            send_control_packet_(CONTROL_PACKET_ACTION_BOOST_CANCEL, 0);
        }

        void iBoostBuddy::setup()
        {
            if (heating_mode_ != nullptr)
            {
                heating_mode_->publish_state("Initializing...");
            }
            if (heating_warn_ != nullptr)
            {
                heating_warn_->publish_state("No Warnings");
            }

            // Initialize numeric sensors
            if (heating_import_)
                heating_import_->publish_state(0);

            if (heating_power_)
                heating_power_->publish_state(0);

            if (heating_today_)
                heating_today_->publish_state(0);

            if (heating_yesterday_)
                heating_yesterday_->publish_state(0);

            if (heating_last_7_)
                heating_last_7_->publish_state(0);

            if (heating_last_28_)
                heating_last_28_->publish_state(0);

            if (heating_last_gt_)
                heating_last_gt_->publish_state(0);

            if (heating_boost_time_)
                heating_boost_time_->publish_state(0);

            ESP_LOGI(TAG_IBOOST, "iBoostBuddy setup (native SX126x) starting");

            if (!radio_)
            {
                ESP_LOGD(TAG_IBOOST, "No SX126x radio linked (radio_id not set)");
            }
            else
            {
                ESP_LOGD(TAG_IBOOST, "SX126x radio linked; radio configuration handled by SX126x component");
            }
        }

        void iBoostBuddy::loop()
        {
            // No manual polling - receiving handled via on_packet lambda calling process_packet().
        }

        void iBoostBuddy::update()
        {
            // Update is Called every 10 seconds by polling component
            send_control_packet_(CONTROL_PACKET_ACTION_REQUEST_DATA, 0);
        }

        void iBoostBuddy::dump_config()
        {
            ESP_LOGCONFIG(TAG_IBOOST, "iBoostBuddy - Configuration Dump");
        }

        void iBoostBuddy::handle_packet_iboost_(const std::vector<uint8_t> &buffer, float rssi)
        {
            if (buffer.size() < 28)
            {
                return; // Packet too short for iBoost data
            }
            // Publish last iBoost RSSI if sensor configured
            if (rssi_iboost_)
                rssi_iboost_->publish_state(rssi);

            // Capture system address from first valid iBoost packet - Unlikely but possible
            if (!is_system_address_valid_)
            {
                system_address_byte0_ = buffer[0];
                system_address_byte1_ = buffer[1];
                is_system_address_valid_ = true;
                system_address_rssi_ = rssi;
                ESP_LOGI(TAG_IBOOST, "System address captured from iBoost: %02X%02X (RSSI=%.1f)", buffer[0], buffer[1], rssi);
            }

            // Validate packet belongs to our system
            if (is_system_address_valid_ &&
                (static_cast<uint8_t>(system_address_byte0_) != buffer[0] ||
                 static_cast<uint8_t>(system_address_byte1_) != buffer[1]))
            {
                ESP_LOGW(TAG_IBOOST, "Received packet from different iBoost system - ignoring");
                return;
            }

            short PowerSentToTank = (*(short *)&buffer[16]);
            long current_import_raw = (*reinterpret_cast<const long *>(&buffer[18]));
            long energy_data_value = (*reinterpret_cast<const long *>(&buffer[25])); // This depends on the request
            uint8_t data_received_mode_id = buffer[24];
            uint8_t boost_time = buffer[5]; // boost time remaining
            bool water_heating = (buffer[6] == 0);
            bool cylinder_hot = (buffer[7] != 0);
            iboost_unit_overheated = (buffer[13] != 0);

            if (heating_mode_) // if linked to a sensor in esphome..
            {
                if (cylinder_hot)
                {
                    heating_mode_->publish_state("OFF: Water Tank Hot");
                    ESP_LOGD(TAG_IBOOST, "Heat: Water Tank Hot");
                }
                else if (iboost_unit_overheated)
                {
                    heating_mode_->publish_state("Failed: Overheat");
                    ESP_LOGD(TAG_IBOOST, "Heat: Failed: Overheat");
                }
                else if (boost_time > 0)
                {
                    heating_mode_->publish_state("ON: Heating from Manual Boost");
                    ESP_LOGD(TAG_IBOOST, "Heat: Heating from Manual Boost");
                }
                else if (water_heating)
                {
                    heating_mode_->publish_state("ON: Heating from Solar");
                    ESP_LOGD(TAG_IBOOST, "Heat: Heating from Solar");
                }
                else
                {
                    heating_mode_->publish_state("OFF: Water Heating Off");
                    ESP_LOGD(TAG_IBOOST, "Heat: Water Heating Off");
                }
            }
            if (heating_warn_) // if linked to a sensor in esphome
            {
                std::string warning_display;

                if (iboost_unit_overheated)
                {
                    warning_display += "iBoost Overheating";
                }

                if (is_sender_battery_low_)
                {
                    if (!warning_display.empty())
                    {
                        warning_display += " | ";
                    }
                    warning_display += "Sender Battery Low";
                }

                if (!warning_display.empty())
                {
                    heating_warn_->publish_state(warning_display);
                    ESP_LOGW(TAG_IBOOST, "Status Warning: %s", warning_display.c_str());
                }
                else
                {
                    heating_warn_->publish_state("");
                }
            }
            if (heating_power_)
            {
                heating_power_->publish_state(PowerSentToTank); // Current power sent to heater
                ESP_LOGV(TAG_IBOOST, "Current Heat Power: %d W", PowerSentToTank);
            }
            if (heating_import_)
            {
                float import_power_watts = static_cast<float>(current_import_raw) / 360.0f;
                heating_import_->publish_state(import_power_watts);
                ESP_LOGV(TAG_IBOOST, "Current Import Power: %.1f W", import_power_watts);
            }
            if (heating_boost_time_)
            {
                heating_boost_time_->publish_state(boost_time);
                ESP_LOGV(TAG_IBOOST, "Boost Time Remaining: %d minutes", boost_time);
            }

            ESP_LOGVV(TAG_IBOOST, "RX: Data received mode ID: %d", data_received_mode_id);
            switch (data_received_mode_id)
            {
            case DATA_REQUEST_TODAY: // 0xCA (202)
                if (heating_today_)
                {
                    heating_today_->publish_state(energy_data_value);
                    ESP_LOGV(TAG_IBOOST, "Received Today's Heating: %ld Wh", energy_data_value);
                }
                break;
            case DATA_REQUEST_YESTERDAY: // 0xCB (203)
                if (heating_yesterday_)
                {
                    heating_yesterday_->publish_state(energy_data_value);
                    ESP_LOGV(TAG_IBOOST, "Received Yesterday's Heating: %ld Wh", energy_data_value);
                }
                break;
            case DATA_REQUEST_LAST_7_DAYS: // 0xCC (204)
                if (energy_data_value > 0 && heating_last_7_)
                {
                    heating_last_7_->publish_state(energy_data_value);
                    ESP_LOGV(TAG_IBOOST, "Received Last 7 Days Heating: %ld Wh", energy_data_value);
                }
                break;
            case DATA_REQUEST_LAST_28_DAYS: // 0xCD (205)
                if (energy_data_value > 0 && heating_last_28_)
                {
                    heating_last_28_->publish_state(energy_data_value);
                    ESP_LOGV(TAG_IBOOST, "Received Last 28 Days Heating: %ld Wh", energy_data_value);
                }
                break;
            case DATA_REQUEST_TOTAL: // 0xCE (206)
                if (energy_data_value > 0 && heating_last_gt_)
                {
                    heating_last_gt_->publish_state(energy_data_value);
                    ESP_LOGV(TAG_IBOOST, "Received Total Heating: %ld Wh", energy_data_value);
                }
                break;
            }
            update_and_publish_packet_count_();
        }

        void iBoostBuddy::handle_packet_buddy_(const std::vector<uint8_t> &buffer, float rssi)
        {
            if (buffer.size() < 28) // Adjusted for no length byte
            {
                return;
            }
            if (rssi_buddy_)
                rssi_buddy_->publish_state(rssi);
            ESP_LOGVV(TAG_IBOOST, "RX: Buddy RSSI:%.1f  Data: %s", rssi, format_hex_pretty(buffer).c_str());
            bool should_update = !is_system_address_valid_ || (rssi > system_address_rssi_);
            if (should_update)
            {
                system_address_byte0_ = buffer[0]; // addr0 is at index 0
                system_address_byte1_ = buffer[1]; // addr1 is at index 1
                is_system_address_valid_ = true;
                system_address_rssi_ = rssi;
                ESP_LOGI(TAG_IBOOST, "RX: System address captured from Buddy: %02X%02X (RSSI=%.1f)", buffer[0], buffer[1], rssi);
            }
            update_and_publish_packet_count_();
        }

        void iBoostBuddy::handle_packet_sender_(const std::vector<uint8_t> &buffer, float rssi)
        {
            if (buffer.size() < 44) // Adjusted for no length byte
            {
                return;
            }
            if (rssi_sender_)
                rssi_sender_->publish_state(rssi);
            ESP_LOGVV(TAG_IBOOST, "RX: Sender RSSI:%.1f  Data: %s", rssi, format_hex_pretty(buffer).c_str());
            bool should_update = !is_system_address_valid_ || (rssi > system_address_rssi_);
            if (should_update)
            {
                system_address_byte0_ = buffer[0]; // addr0 is at index 0
                system_address_byte1_ = buffer[1]; // addr1 is at index 1
                is_system_address_valid_ = true;
                system_address_rssi_ = rssi;
                ESP_LOGI(TAG_IBOOST, "RX: System address captured from Sender: %02X%02X (RSSI=%.1f)", buffer[0], buffer[1], rssi);
            }

            is_sender_battery_low_ = (buffer[12] == 0x01); // Battery status from sender packet
            update_and_publish_packet_count_();
        }

        void iBoostBuddy::process_packet(const std::vector<uint8_t> &x, float rssi)
        {
            if (x.size() < 3)
            { // addr0 + addr1 + packet_type
                ESP_LOGW(TAG_IBOOST, "RX: Packet too short: %zu bytes", x.size());
                return;
            }
            // Extract packet info
            uint8_t packet_length = x.size();
            uint8_t packet_type = x[2];

            ESP_LOGVV(TAG_IBOOST, "RX: Processing packet: Length=0x%02X, Type=0x%02X, RSSI=%.1f", packet_length, packet_type, rssi);
            ESP_LOGVV(TAG_IBOOST, "RX: Raw Data : %s", format_hex_pretty(x).c_str());

            // Validate packet length constraints
            if (packet_length < 10 || packet_length > 62)
            {
                ESP_LOGW(TAG_IBOOST, "RX: Invalid packet length: %d bytes", packet_length);
                return;
            }

            switch (packet_type)
            {
            case PACKET_TYPE_IBOOST:
                ESP_LOGD(TAG_IBOOST, "RX: Found iBoost packet - sending to packet decoder");
                handle_packet_iboost_(x, rssi);
                break;

            case PACKET_TYPE_BUDDY: // should check is len 29
                ESP_LOGVV(TAG_IBOOST, "RX: Found Buddy packet - sending to packet decoder");
                handle_packet_buddy_(x, rssi);
                break;

            case PACKET_TYPE_SENDER: // should check is len 44
                ESP_LOGVV(TAG_IBOOST, "RX: Found Sender packet - sending to packet decoder");
                handle_packet_sender_(x, rssi);
                break;

            default:
                ESP_LOGW(TAG_IBOOST, "RX: Unknown packet type: 0x%02X", packet_type);
                break;
            }
        }

    } // namespace esphiBoost
} // namespace esphome
