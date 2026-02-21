#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include <string>

namespace esphome {
  namespace jura_coffee {

    class JuraCoffee : public PollingComponent, public uart::UARTDevice {
    public:
      explicit JuraCoffee(uart::UARTComponent *parent);

      void setup() override;
      void loop() override;
      void update() override;
      void dump_config() override;

      float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

      void cmd2jura(const std::string &cmd);

      void set_single_espresso_sensor(sensor::Sensor *s) { single_espresso_sensor_ = s; }
      void set_double_espresso_sensor(sensor::Sensor *s) { double_espresso_sensor_ = s; }
      void set_coffee_sensor(sensor::Sensor *s) { coffee_sensor_ = s; }
      void set_double_coffee_sensor(sensor::Sensor *s) { double_coffee_sensor_ = s; }
      void set_clean_sensor(sensor::Sensor *s) { clean_sensor_ = s; }
      void set_tray_status_sensor(text_sensor::TextSensor *s) { tray_status_sensor_ = s; }
      void set_bean_status_sensor(text_sensor::TextSensor *s) { bean_status_sensor_ = s; }
      void set_tank_status_sensor(text_sensor::TextSensor *s) { tank_status_sensor_ = s; }
      void set_machine_type_sensor(text_sensor::TextSensor *s) { machine_type_sensor_ = s; }
      void set_hz_raw_sensor(text_sensor::TextSensor *s) { hz_raw_sensor_ = s; }
      void set_coffee_temperature_sensor(sensor::Sensor *s) { coffee_temperature_sensor_ = s; }
      void set_steam_temperature_sensor(sensor::Sensor *s) { steam_temperature_sensor_ = s; }
      void set_brew_mode_sensor(text_sensor::TextSensor *s) { brew_mode_sensor_ = s; }
      void set_is_brewing_sensor(binary_sensor::BinarySensor *s) { is_brewing_sensor_ = s; }

    protected:
      bool ends_with(const std::string &str, const std::string &suffix) const;
      void process_response(const std::string &response);

      sensor::Sensor *single_espresso_sensor_{nullptr};
      sensor::Sensor *double_espresso_sensor_{nullptr};
      sensor::Sensor *coffee_sensor_{nullptr};
      sensor::Sensor *double_coffee_sensor_{nullptr};
      sensor::Sensor *clean_sensor_{nullptr};
      sensor::Sensor *coffee_temperature_sensor_{nullptr};
      sensor::Sensor *steam_temperature_sensor_{nullptr};
      text_sensor::TextSensor *tray_status_sensor_{nullptr};
      text_sensor::TextSensor *bean_status_sensor_{nullptr};
      text_sensor::TextSensor *tank_status_sensor_{nullptr};
      text_sensor::TextSensor *machine_type_sensor_{nullptr};
      text_sensor::TextSensor *hz_raw_sensor_{nullptr};
      text_sensor::TextSensor *brew_mode_sensor_{nullptr};
      binary_sensor::BinarySensor *is_brewing_sensor_{nullptr};

      uint32_t num_single_espresso_{0};
      uint32_t num_double_espresso_{0};
      uint32_t num_coffee_{0};
      uint32_t num_double_coffee_{0};
      uint32_t num_clean_{0};

      std::string tray_status_;
      std::string bean_status_;
      std::string tank_status_;

      int flip_{0};
      size_t command_pos_{0};
      std::string data_{};
      std::string command_{};

      bool ty_sent_on_startup_{false};
      bool startup_delay_done_{false};
    };

  }  // namespace jura_coffee
}  // namespace esphome
