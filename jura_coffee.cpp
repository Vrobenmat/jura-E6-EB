#include "esphome.h"
#include "jura_coffee.h"
#include "esphome/core/log.h"

#include <vector>
#include <sstream>

namespace esphome {
    namespace jura_coffee {

        static const char *const TAG = "jura_coffee";

        JuraCoffee::JuraCoffee(uart::UARTComponent *parent) : PollingComponent(10000), UARTDevice(parent) {}

        void JuraCoffee::cmd2jura(const std::string &cmd) { command_ = cmd; }

        bool JuraCoffee::ends_with(const std::string &str, const std::string &suffix) const {
            if (suffix.size() > str.size()) return false;
            return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        void JuraCoffee::setup() {
            // Wait 5 seconds after ESPHome boot before talking to the Jura
            this->set_timeout("jura_startup_delay", 5000, [this]() {
                this->startup_delay_done_ = true;
                ESP_LOGI(TAG, "5-second startup delay completed → starting Jura E6 communication");
                this->update();  // manually trigger the very first update (TY: + cycle)
            });
        }

        void JuraCoffee::loop() {
            // === Sending phase (non-blocking, one character per loop iteration) ===
            if (!command_.empty() && data_.empty()) {
                if (command_pos_ == 0) {
                    command_ += "\r\n";
                }
                if (command_pos_ < command_.length()) {
                    uint8_t currentbyte = static_cast<uint8_t>(command_[command_pos_]);
                    std::bitset<8> current_bits{currentbyte};

                    for (int s = 0; s < 8; s += 2) {
                        std::bitset<8> rawbyte{0x7F};
                        rawbyte.set(2, current_bits[s]);
                        rawbyte.set(5, current_bits[s + 1]);
                        write_byte(static_cast<uint8_t>(rawbyte.to_ulong()));
                        this->flush();
                    }
                    ++command_pos_;
                    if (command_pos_ >= command_.length()) {
                        command_.clear();
                        command_pos_ = 0;
                    }
                }
                return;
            }

            if (!available()) return;

            // === Receiving phase ===
            std::string inbytes;
            uint8_t inbyte = 0;
            int bit_pos = 0;

            while (available()) {
                uint8_t raw = read();
                //bitWrite(inbyte, bit_pos + 0, bitRead(raw, 2));
                //bitWrite(inbyte, bit_pos + 1, bitRead(raw, 5));
                inbyte |= ((raw >> 2) & 1U) << bit_pos;
                inbyte |= ((raw >> 5) & 1U) << (bit_pos + 1);

                if ((bit_pos += 2) >= 8) {
                    bit_pos = 0;
                    inbytes += static_cast<char>(inbyte);
                    inbyte = 0;
                }
            }

            data_ += inbytes;

            if (ends_with(data_, "\r\n")) {
                ESP_LOGD(TAG, "Received: %s", data_.c_str());
                data_.resize(data_.length() - 2);
                process_response(data_);
                data_.clear();
            }
        }

        void JuraCoffee::process_response(const std::string &response) {

            if (response.compare(0, 3, "hz:") == 0) {
                ESP_LOGD(TAG, "HZ response: %s", response.c_str());

                if (hz_raw_sensor_) hz_raw_sensor_->publish_state(response);

                // Parse fields
                std::vector<std::string> fields;
                std::istringstream ss(response.substr(3));
                std::string token;
                while (std::getline(ss, token, ',')) {
                    fields.push_back(token);
                }
                if (fields.size() >= 7) {
                    uint16_t coffee_raw = std::stoul(fields[4], nullptr, 16);
                    uint16_t steam_raw  = std::stoul(fields[5], nullptr, 16);
                    std::string mode_hex = fields[6];

                    float coffee_temp = coffee_raw * 0.09f;
                    float steam_temp  = steam_raw * 0.09f;

                    if (coffee_temperature_sensor_) coffee_temperature_sensor_->publish_state(coffee_temp);
                    if (steam_temperature_sensor_)  steam_temperature_sensor_->publish_state(steam_temp);

                    std::string mode_str = "Unknown";
                    if (mode_hex == "3") mode_str = "Normal";
                    else if (mode_hex == "5") mode_str = "Cappuccino Coffee";
                    else if (mode_hex == "6") mode_str = "Cappuccino Milk";
                    if (brew_mode_sensor_) brew_mode_sensor_->publish_state(mode_str);

                    bool brewing = (mode_hex != "3");
                    if (is_brewing_sensor_) is_brewing_sensor_->publish_state(brewing);
                }
                return;
            }

            if (response.compare(0, 3, "ty:") == 0) {                     // ← NEW
                std::string value = response.substr(3);
                ESP_LOGD(TAG, "Machine type: %s", value.c_str());
                if (machine_type_sensor_) machine_type_sensor_->publish_state(value);
                return;
            }

            if (response.compare(0, 3, "rt:") == 0) {
                std::string sub;
                if (flip_ == 1) {
                    sub = response.substr(3, 4);  num_single_espresso_ = std::stoul(sub, nullptr, 16);
                    sub = response.substr(11, 4); num_coffee_ = std::stoul(sub, nullptr, 16);
                    sub = response.substr(35, 4); num_clean_ = std::stoul(sub, nullptr, 16);

                    if (single_espresso_sensor_) single_espresso_sensor_->publish_state(num_single_espresso_);
                    if (coffee_sensor_) coffee_sensor_->publish_state(num_coffee_);
                    if (clean_sensor_) clean_sensor_->publish_state(num_clean_);
                } else if (flip_ == 2) {
                    sub = response.substr(63, 4); num_double_espresso_ = std::stoul(sub, nullptr, 16);
                    sub = response.substr(7, 4);  num_double_coffee_ = std::stoul(sub, nullptr, 16);

                    if (double_espresso_sensor_) double_espresso_sensor_->publish_state(num_double_espresso_);
                    if (double_coffee_sensor_) double_coffee_sensor_->publish_state(num_double_coffee_);
                }
            } else if (response.compare(0, 3, "ic:") == 0) {
                uint8_t b1 = static_cast<uint8_t>(std::stoul(response.substr(3, 2), nullptr, 16));
                uint8_t b2 = static_cast<uint8_t>(std::stoul(response.substr(5, 2), nullptr, 16));

                tray_status_ = ((b1 >> 4) & 1U) == 0 ? "Missing" : "Present";
                bean_status_ = ((b1 >> 2) & 1U) == 0 ? "Add Beans" : "OK";
                tank_status_ = ((b2 >> 5) & 1U) == 1 ? "Fill Tank" : "OK";

                if (tray_status_sensor_) tray_status_sensor_->publish_state(tray_status_);
                if (bean_status_sensor_) bean_status_sensor_->publish_state(bean_status_);
                if (tank_status_sensor_) tank_status_sensor_->publish_state(tank_status_);
            } else {
                ESP_LOGD(TAG, "Unexpected response: %s", response.c_str());
            }
        }

        void JuraCoffee::update() {

            if (!this->startup_delay_done_) {
                return;  // still in delay → do nothing
            }

            ESP_LOGD(TAG, "Starting update cycle");

            if (!ty_sent_on_startup_) {
                ESP_LOGD(TAG, "Sending one-off TY: on startup");
                command_ = "TY:";
                ty_sent_on_startup_ = true;
                return;  // skip normal cycle this time
            }

            switch (flip_) {
                case 0: command_ = "RT:0000"; break;
                case 1: command_ = "RT:0010"; break;
                case 2: command_ = "IC:"; break;
                case 3: command_ = "HZ:"; break;
            }
            flip_ = (flip_ + 1) % 4;
        }

        void JuraCoffee::dump_config() {
            ESP_LOGCONFIG(TAG, "Jura Coffee:");
            ESP_LOGCONFIG(TAG, "  Update interval: %" PRIu32 " ms", get_update_interval());
            LOG_SENSOR("  ", "Single Espresso", single_espresso_sensor_);
            LOG_SENSOR("  ", "Double Espresso", double_espresso_sensor_);
            LOG_SENSOR("  ", "Coffee", coffee_sensor_);
            LOG_SENSOR("  ", "Double Coffee", double_coffee_sensor_);
            LOG_SENSOR("  ", "Cleanings", clean_sensor_);
            LOG_TEXT_SENSOR("  ", "Machine Type", machine_type_sensor_);
            LOG_TEXT_SENSOR("  ", "Tray", tray_status_sensor_);
            LOG_TEXT_SENSOR("  ", "Beans", bean_status_sensor_);
            LOG_TEXT_SENSOR("  ", "Tank", tank_status_sensor_);
        }

    }  // namespace jura_coffee
}  // namespace esphome
