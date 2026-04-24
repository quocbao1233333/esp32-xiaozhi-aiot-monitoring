# Software Flow

## 1. Setup Stage

When the ESP32 starts, the firmware performs the following tasks:

1. Start Serial Monitor at 115200 baud.
2. Configure LED and buzzer pins as output.
3. Turn off LED and buzzer at startup.
4. Initialize DHT11 sensor.
5. Initialize OLED display.
6. Connect to Wi-Fi.
7. Connect to XiaoZhi MCP server.
8. Register chatbot tools.

## 2. Main Loop

The main loop performs the following tasks:

1. Maintain MCP connection by calling `mcpClient.loop()`.
2. Read current time using `millis()`.
3. Read DHT11 data every 2000 ms.
4. Update OLED display.
5. Check alarm condition.
6. If temperature is higher than threshold or AI manually turns on alarm, blink LED and buzzer every 300 ms.
7. Otherwise, turn off LED and buzzer.

## 3. Alarm Logic

The alarm condition is:

```cpp
bool isAlarming = (temperature > warningTemp) || (manualAlarm == true);
This means the alarm can be activated by either high temperature or remote AI command.


Trong bài báo, thuật toán cũng được mô tả gồm `Setup`, `Loop`, cơ chế non-blocking, đọc cảm biến mỗi 2 giây, ngưỡng cảnh báo mặc định 35°C và nhấp nháy LED/còi theo chu kỳ 300ms. :contentReference[oaicite:8]{index=8} Code của bạn cũng triển khai đúng logic này trong `loop()`. :contentReference[oaicite:9]{index=9}

---

## 4. File `docs/chatbot-tools.md`

```markdown
# XiaoZhi Chatbot Tools

The ESP32 firmware registers three tools for XiaoZhi chatbot.

## 1. temperature_and_humidity_values

This tool returns current temperature and humidity values from the DHT11 sensor.

Returned JSON example:

```json
{
  "temperature": 30.5,
  "humidity": 70.0
}
2. set_temperature_warning

This tool updates the maximum temperature threshold.

Input JSON example:

{
  "max_temperature": 35
}

Returned JSON example:

{
  "success": true,
  "new_limit": 35
}
3. control_alarm

This tool turns the warning LED and buzzer on or off.

Input JSON example:

{
  "state": "on"
}

or:

{
  "state": "off"
}

Returned JSON example:

{
  "success": true,
  "state": "off"
}

Code của bạn đăng ký đúng 3 tool: `temperature_and_humidity_values`, `set_temperature_warning`, và `control_alarm`. :contentReference[oaicite:10]{index=10}

---

## 5. File `docs/test-scenarios.md`

```markdown
# Test Scenarios

## Scenario 1: Temperature and Humidity Query

### User command

```text
What is the current temperature?
Expected behavior
XiaoZhi receives the user command.
XiaoZhi calls temperature_and_humidity_values.
ESP32 returns temperature and humidity data.
XiaoZhi replies to the user.
OLED displays the same values.
Scenario 2: Turn On Alarm
User command
Turn on the alarm.
Expected behavior
XiaoZhi calls control_alarm.
The input state is on.
ESP32 sets manualAlarm = true.
LED and buzzer blink every 300 ms.
Scenario 3: Turn Off Alarm
User command
Turn off the alarm.
Expected behavior
XiaoZhi calls control_alarm.
The input state is off.
ESP32 sets manualAlarm = false.
LED and buzzer turn off if temperature is below warning threshold.
Scenario 4: Change Temperature Warning Threshold
User command
Set warning temperature to 32 degrees.
Expected behavior
XiaoZhi calls set_temperature_warning.
ESP32 updates warningTemp.
OLED displays the new threshold.

Bài báo có hai nhóm thực nghiệm chính: truy xuất nhiệt độ/độ ẩm qua XiaoZhi và điều khiển tắt/bật đèn/còi cảnh báo bằng lệnh tự nhiên. :contentReference[oaicite:11]{index=11}

---

## 6. File `docs/flowchart.md`

```markdown
# Software Flowchart

The software flowchart describes the firmware behavior of the ESP32 system.

```mermaid
flowchart TD
    A([Start]) --> B[Initialize Serial 115200]
    B --> C[Configure LED and Buzzer pins]
    C --> D[Turn off LED and Buzzer]
    D --> E[Initialize DHT11 sensor]
    E --> F{OLED initialized?}

    F -- No --> G[Print OLED ERROR and stop]
    F -- Yes --> H[Clear OLED display]
    H --> I[Connect to WiFi]
    I --> J{WiFi connected?}

    J -- No --> K[Wait 200 ms and retry]
    K --> J
    J -- Yes --> L[Connect to XiaoZhi MCP Server]
    L --> M[Enter main loop]

    M --> N[Run mcpClient.loop]
    N --> O[Get current time with millis]
    O --> P{2000 ms elapsed?}

    P -- Yes --> Q[Read temperature from DHT11]
    Q --> R[Read humidity from DHT11]
    R --> S[Update OLED display]
    S --> T{Temperature high or AI alarm on?}

    P -- No --> T

    T -- Yes --> U{300 ms elapsed?}
    U -- Yes --> V[Toggle ledState]
    V --> W[Write ledState to LED and Buzzer]
    W --> N

    U -- No --> N

    T -- No --> X[Turn off LED]
    X --> Y[Turn off Buzzer]
    Y --> Z[Set ledState false]
    Z --> N

---

# VIII. Tạo file README riêng trong firmware

Tạo file:

```text
firmware/esp32_xiaozhi_aiot/README.md

Dán:

# ESP32 XiaoZhi AIoT Firmware

## 1. Description

This folder contains the Arduino firmware for the ESP32-based AIoT monitoring and alarm control system.

## 2. Main Firmware File

```text
esp32_xiaozhi_aiot.ino
3. Configuration

Before compiling, create a local config.h file from config.example.h.

cp config.example.h config.h

Then update:

Wi-Fi SSID
Wi-Fi password
XiaoZhi MCP endpoint
Pin configuration if needed
4. Arduino IDE Settings
Board: ESP32 Dev Module
Baud rate: 115200
Upload speed: default or 115200
Serial Monitor baud rate: 115200
5. Required Libraries
WiFi
WebSocketMCP
ArduinoJson
DHT sensor library
SPI
Adafruit GFX
Adafruit SSD1306

---

# IX. Chuẩn bị hình ảnh đưa vào GitHub

Trong `assets/images/`, bạn nên để các ảnh sau:

```text
system-block-diagram.png
hardware-wiring.png
software-flowchart.png
oled-temperature-demo.jpg
xiaozhi-listening-demo.jpg
alarm-control-demo.jpg
final-hardware.jpg

Cách đặt ảnh từ bài báo sang GitHub:

Mở file PDF bài báo.
Chụp/cắt từng hình:
Hình sơ đồ hệ thống.
Hình thiết kế phần cứng.
Hình lưu đồ thuật toán.
Hình OLED hiển thị nhiệt độ/độ ẩm.
Hình chatbot đang lắng nghe.
Hình LED/còi cảnh báo.
Lưu đúng tên trong assets/images/.

Trong bài báo, trang phần kết quả có các hình minh họa quá trình chạy Arduino IDE, chatbot nghe lệnh, hiển thị đồng thời trên OLED và chatbot, điều khiển LED/còi.
