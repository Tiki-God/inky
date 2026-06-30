#include "inky_impression_4.h"
#include "esphome/core/log.h"
#include <algorithm>

namespace esphome {
namespace inky_impression_4 {

static const char *const TAG = "inky_impression_4";

static uint8_t color_to_palette(Color color) {
  struct PaletteColor {
    uint8_t r, g, b;
    uint8_t index;
  };
  
  static const PaletteColor palette[] = {
    {57, 48, 57, 0},     // BLACK
    {255, 255, 255, 1}, // WHITE
    {58, 91, 70, 2},    // GREEN
    {61, 59, 94, 3},    // BLUE
    {156, 72, 75, 4},   // RED
    {208, 190, 71, 5},  // YELLOW
    {177, 106, 73, 6},  // ORANGE
  };

  uint8_t r = color.red;
  uint8_t g = color.green;
  uint8_t b = color.blue;
  
  uint32_t min_dist = 999999;
  uint8_t closest_index = 1; // Default to white
  
  for (const auto& p : palette) {
    int32_t dr = (int32_t)r - p.r;
    int32_t dg = (int32_t)g - p.g;
    int32_t db = (int32_t)b - p.b;
    uint32_t dist = dr*dr + dg*dg + db*db;
    if (dist < min_dist) {
      min_dist = dist;
      closest_index = p.index;
    }
  }
  return closest_index;
}

void InkyImpression4::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Inky Impression 4 display...");
  
  // Set Pin Modes
  this->dc_pin_->setup();
  this->reset_pin_->setup();
  this->busy_pin_->setup();
  
  // Initialize SPI
  this->spi_setup();
  
  // Allocate buffer (640 * 400 / 2 = 128,000 bytes)
  this->buffer_ = new uint8_t[128000];
  if (this->buffer_ == nullptr) {
    ESP_LOGE(TAG, "Failed to allocate display buffer!");
    this->mark_failed();
    return;
  }
  // Initialize to White (1) -> (1 << 4) | 1 = 0x11
  memset(this->buffer_, 0x11, 128000);

  // Perform Hardware Reset
  this->reset_();
  
  // 1. Resolution Setting (TRES = 0x61)
  // width = 640 (0x0280), height = 400 (0x0190)
  uint8_t tres_data[4] = {0x02, 0x80, 0x01, 0x90};
  this->write_command_(0x61);
  this->write_data_(tres_data, 4);
  
  // 2. Panel Setting (PSR = 0x00)
  // resolution_setting = 0b10 (for 640x400)
  // data[0] = (resolution_setting << 6) | 0b101111 = 0xAF
  // data[1] = 0x08 (7-color mode)
  uint8_t psr_data[2] = {0xAF, 0x08};
  this->write_command_(0x00);
  this->write_data_(psr_data, 2);
  
  // 3. Power Settings (PWR = 0x01)
  uint8_t pwr_data[4] = {0x37, 0x00, 0x23, 0x23};
  this->write_command_(0x01);
  this->write_data_(pwr_data, 4);
  
  // 4. PLL clock frequency (PLL = 0x30)
  this->write_command_(0x30);
  this->write_data_(0x3C);
  
  // 5. TSE register (TSE = 0x41)
  this->write_command_(0x41);
  this->write_data_(0x00);
  
  // 6. VCOM and Data Interval setting (CDI = 0x50)
  // cdi = (border_color << 5) | 0x17
  uint8_t cdi_data = (this->border_color_ << 5) | 0x17;
  this->write_command_(0x50);
  this->write_data_(cdi_data);
  
  // 7. Gate/Source non-overlap period (TCON = 0x60)
  this->write_command_(0x60);
  this->write_data_(0x22);
  
  // 8. Disable external flash (DAM = 0x65)
  this->write_command_(0x65);
  this->write_data_(0x00);
  
  // 9. Power Setting (PWS = 0xE3)
  this->write_command_(0xE3);
  this->write_data_(0xAA);
  
  // 10. Power off sequence (PFS = 0x03)
  this->write_command_(0x03);
  this->write_data_(0x00);
}

void InkyImpression4::dump_config() {
  LOG_DISPLAY("", "Inky Impression 4\"", this);
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  LOG_PIN("  Busy Pin: ", this->busy_pin_);
  ESP_LOGCONFIG(TAG, "  Border Color Code: %d", this->border_color_);
}

void InkyImpression4::update() {
  if (this->buffer_ == nullptr) {
    return;
  }

  // 1. Call the lambda writer if one is configured
  this->do_update_();
  
  // 2. Send command to update display (DTM1 = 0x10)
  this->write_command_(0x10);
  this->write_data_(this->buffer_, 128000);
  
  // 3. Power ON (PON = 0x04)
  this->write_command_(0x04);
  delay(200); // 0.2s
  
  // 4. Display Refresh (DRF = 0x12)
  this->write_command_(0x12);
  this->wait_until_idle_();
  
  // 5. Power OFF (POF = 0x02)
  this->write_command_(0x02);
  delay(200); // 0.2s
}

void InkyImpression4::draw_absolute_pixel_t(int x, int y, Color color) {
  if (this->buffer_ == nullptr) return;
  if (x < 0 || x >= 640 || y < 0 || y >= 400) {
    return;
  }
  
  uint8_t pixel_val = color_to_palette(color);
  int pixel_index = y * 640 + x;
  int byte_index = pixel_index / 2;
  
  if (pixel_index % 2 == 0) {
    this->buffer_[byte_index] &= 0x0F;
    this->buffer_[byte_index] |= (pixel_val << 4) & 0xF0;
  } else {
    this->buffer_[byte_index] &= 0xF0;
    this->buffer_[byte_index] |= pixel_val & 0x0F;
  }
}

void InkyImpression4::write_command_(uint8_t value) {
  this->dc_pin_->digital_write(false); // Command
  this->enable();
  this->write_byte(value);
  this->disable();
}

void InkyImpression4::write_data_(uint8_t value) {
  this->dc_pin_->digital_write(true); // Data
  this->enable();
  this->write_byte(value);
  this->disable();
}

void InkyImpression4::write_data_(const uint8_t *buffer, size_t length) {
  this->dc_pin_->digital_write(true); // Data
  this->enable();
  
  size_t offset = 0;
  while (offset < length) {
    size_t chunk_size = std::min((size_t)4096, length - offset);
    this->write_array(buffer + offset, chunk_size);
    offset += chunk_size;
    yield();
  }
  
  this->disable();
}

bool InkyImpression4::wait_until_idle_() {
  uint32_t start_time = millis();
  while (this->busy_pin_->digital_read() == false) {
    if (millis() - start_time > 40000) {
      ESP_LOGW(TAG, "Timeout waiting for display to be idle");
      return false;
    }
    delay(1);
  }
  return true;
}

void InkyImpression4::reset_() {
  this->reset_pin_->digital_write(false); // LOW to reset
  delay(100);
  this->reset_pin_->digital_write(true);  // HIGH to release
  delay(100);
  this->wait_until_idle_();
}

}  // namespace inky_impression_4
}  // namespace esphome
