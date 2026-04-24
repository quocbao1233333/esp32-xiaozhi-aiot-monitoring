# Hardware Design

## 1. Components

| Component | Quantity | Description |
|---|---:|---|
| ESP32 | 1 | Main controller |
| DHT11 | 1 | Temperature and humidity sensor |
| OLED SSD1306 SPI | 1 | Display module |
| LED | 1 | Warning indicator |
| Buzzer | 1 | Audio alarm |
| Breadboard | 1 | Prototype platform |
| Jumper wires | Several | Circuit connection |

## 2. Wiring Table

| Device | ESP32 Pin |
|---|---|
| DHT11 DATA | GPIO 23 |
| OLED MOSI | GPIO 19 |
| OLED CLK | GPIO 18 |
| OLED DC | GPIO 21 |
| OLED CS | GPIO 22 |
| OLED RESET | GPIO 17 |
| Warning LED | GPIO 5 |
| Buzzer | GPIO 4 |

## 3. Hardware Notes

- The DHT11 sensor is used for temperature and humidity measurement.
- The OLED module is used to display temperature, humidity, threshold, and warning state.
- The LED and buzzer are used as alarm outputs.
- The ESP32 provides Wi-Fi communication for cloud and chatbot integration.
- Current-limiting resistors should be used with LEDs.
