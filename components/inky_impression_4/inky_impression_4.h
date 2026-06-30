#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
namespace inky_impression_4 {

enum BorderColor {
  BORDER_COLOR_BLACK = 0,
  BORDER_COLOR_WHITE = 1,
  BORDER_COLOR_GREEN = 2,
  BORDER_COLOR_BLUE = 3,
  BORDER_COLOR_RED = 4,
  BORDER_COLOR_YELLOW = 5,
  BORDER_COLOR_ORANGE = 6,
  BORDER_COLOR_CLEAN = 7,
};

class InkyImpression4 : public PollingComponent, public display::DisplayBuffer, public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW, spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_4MHZ> {
 public:
  void set_dc_pin(GPIOPin *dc_pin) { dc_pin_ = dc_pin; }
  void set_reset_pin(GPIOPin *reset_pin) { reset_pin_ = reset_pin; }
  void set_busy_pin(GPIOPin *busy_pin) { busy_pin_ = busy_pin; }
  void set_border_color(uint8_t color) { border_color_ = color; }

  void setup() override;
  void dump_config() override;
  void update() override;

  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  display::DisplayType get_display_type() override { return display::DISPLAY_TYPE_COLOR; }

 protected:
  void draw_absolute_pixel_t(int x, int y, Color color) override;
  int get_width_internal() override { return 640; }
  int get_height_internal() override { return 400; }

  void write_command_(uint8_t value);
  void write_data_(uint8_t value);
  void write_data_(const uint8_t *buffer, size_t length);
  bool wait_until_idle_();
  void reset_();

  GPIOPin *dc_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  GPIOPin *busy_pin_{nullptr};
  uint8_t border_color_{1}; // BORDER_COLOR_WHITE

  uint8_t *buffer_{nullptr};
};

}  // namespace inky_impression_4
}  // namespace esphome
