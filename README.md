# ESPHome Component: Inky Impression 4"

A custom ESPHome external component for the **Pimoroni Inky Impression 4"** 7-color e-paper display (640x400 resolution, driven by the **UC8159** controller).

This component is converted from the official Pimoroni Python library and allows you to render rich graphics, shapes, and text on the display using ESPHome's native drawing framework.

## Features

- **Standard ESPHome Display Interface**: Full support for drawing primitives (`it.print`, `it.line`, `it.filled_rectangle`, etc.).
- **Automatic Color Mapping**: Standard RGB colors are automatically matched to the closest supported physical color on the display using Euclidean color distance.
- **Watchdog-Friendly SPI Writes**: Large buffer transfers (128KB) are chunked in 4KB blocks with periodic yielding to avoid triggering system watchdogs on the ESP32.
- **Configurable Border**: Supports setting the border color of the display.

---

## Configuration Example

Below is a configuration template showing how to pull the custom component from your GitHub repository and initialize it in your ESPHome YAML.

```yaml
esphome:
  name: inky-display-node

esp32:
  board: esp32dev
  framework:
    type: arduino

# Enable logging
logger:
  level: INFO

# Configure the SPI Bus
spi:
  clk_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19

# Pull the component from the GitHub repository
external_components:
  - source:
      type: git
      url: https://github.com/Tiki-God/inky
      ref: master
    components: [ inky_impression_4 ]

font:
  - file: "gfont://Roboto"
    id: font_roboto
    size: 24

# Initialize the display platform
display:
  - platform: inky_impression_4
    id: my_display
    cs_pin: GPIO14
    dc_pin: GPIO22
    reset_pin: GPIO21
    busy_pin: GPIO17
    border_color: orange   # Options: black, white, green, blue, red, yellow, orange, clean
    update_interval: 60s
    lambda: |-
      // Clear the screen with White
      it.fill(Color(255, 255, 255));
      
      // Print some text
      it.print(10, 10, id(font_roboto), Color(57, 48, 57), "Inky Impression 4");
      
      // Draw rectangles for all 7 supported colors
      it.filled_rectangle(10, 80, 50, 50, Color(57, 48, 57));     // Black
      it.filled_rectangle(70, 80, 50, 50, Color(255, 255, 255));  // White
      it.rectangle(70, 80, 50, 50, Color(57, 48, 57));            // Outline for White
      it.filled_rectangle(130, 80, 50, 50, Color(58, 91, 70));    // Green
      it.filled_rectangle(190, 80, 50, 50, Color(61, 59, 94));    // Blue
      it.filled_rectangle(250, 80, 50, 50, Color(156, 72, 75));   // Red
      it.filled_rectangle(310, 80, 50, 50, Color(208, 190, 71));  // Yellow
      it.filled_rectangle(370, 80, 50, 50, Color(177, 106, 73));  // Orange
```

---

## Supported Color Palette

The C++ driver maps any drawing colors to one of the 7 supported colors based on the minimum Euclidean distance in RGB space:

| Code Value | Color Name | RGB Saturated Values |
|:---|:---|:---|
| `0` | BLACK | `Color(57, 48, 57)` |
| `1` | WHITE | `Color(255, 255, 255)` |
| `2` | GREEN | `Color(58, 91, 70)` |
| `3` | BLUE | `Color(61, 59, 94)` |
| `4` | RED | `Color(156, 72, 75)` |
| `5` | YELLOW | `Color(208, 190, 71)` |
| `6` | ORANGE | `Color(177, 106, 73)` |
