An ESP32-based AIoT system for real-time temperature and humidity monitoring, OLED display, alarm control, and XiaoZhi chatbot interaction.
# ESP32 XiaoZhi AIoT Monitoring and Alarm Control System

## 1. Project Overview

This project presents an AIoT system based on the ESP32 microcontroller and XiaoZhi chatbot. The system can monitor environmental data, display temperature and humidity on an OLED screen, and control alarm devices such as LED and buzzer through natural language interaction.

The main objective is to build a low-cost smart monitoring and control prototype for smart home and AIoT applications.

## 2. Key Features

- Real-time temperature and humidity monitoring using DHT11.
- OLED display for local data visualization.
- LED and buzzer alarm control through ESP32 GPIO.
- XiaoZhi chatbot integration through WebSocket MCP.
- Natural language interaction for reading sensor values and controlling alarm state.
- Temperature warning threshold configuration through chatbot command.
- Non-blocking sensor reading every 2 seconds.
- Alarm blinking every 300 ms when warning condition is active.

## 3. System Architecture

![System Block Diagram](assets/images/system-block-diagram.png)

The system consists of four main blocks:

1. Input block: DHT11 temperature and humidity sensor.
2. Processing block: ESP32 microcontroller.
3. Output block: OLED display, warning LED, and buzzer.
4. Communication block: Wi-Fi, cloud server, and XiaoZhi chatbot.

## 4. Hardware Components

| Component | Function |
|---|---|
| ESP32 | Main controller and Wi-Fi communication |
| DHT11 | Temperature and humidity measurement |
| OLED SSD1306 SPI | Display temperature, humidity, and warning status |
| LED | Visual alarm indicator |
| Buzzer | Audio alarm indicator |
| Resistors | Pull-up and current-limiting protection |
| Breadboard and jumper wires | Prototype connection |

## 5. Pin Configuration

| Module | ESP32 Pin |
|---|---|
| OLED MOSI | GPIO 19 |
| OLED CLK | GPIO 18 |
| OLED DC | GPIO 21 |
| OLED CS | GPIO 22 |
| OLED RESET | GPIO 17 |
| DHT11 DATA | GPIO 23 |
| Warning LED | GPIO 5 |
| Buzzer | GPIO 4 |

## 6. Software Requirements

- Arduino IDE
- ESP32 board package
- WiFi library
- WebSocketMCP library
- ArduinoJson library
- DHT sensor library
- Adafruit GFX library
- Adafruit SSD1306 library

## 7. Repository Structure

```text
esp32-xiaozhi-aiot-monitoring/
│
├── README.md
├── LICENSE
├── .gitignore
├── firmware/
│   └── esp32_xiaozhi_aiot/
│       ├── esp32_xiaozhi_aiot.ino
│       ├── config.example.h
│       └── README.md
├── docs/
│   ├── architecture.md
│   ├── hardware.md
│   ├── software-flow.md
│   ├── chatbot-tools.md
│   ├── test-scenarios.md
│   └── flowchart.md
├── assets/
│   ├── images/
│   └── videos/
└── report/
    └── nhom2_csudiot_jte.pdf

8. How to Run the Project
Step 1: Clone the repository
git clone https://github.com/YOUR_USERNAME/esp32-xiaozhi-aiot-monitoring.git
cd esp32-xiaozhi-aiot-monitoring
Step 2: Create local configuration file

Copy the example configuration file:

cp firmware/esp32_xiaozhi_aiot/config.example.h firmware/esp32_xiaozhi_aiot/config.h

Then open config.h and fill in your Wi-Fi SSID, Wi-Fi password, and XiaoZhi MCP endpoint.

Step 3: Open the firmware in Arduino IDE

Open this file:

firmware/esp32_xiaozhi_aiot/esp32_xiaozhi_aiot.ino
Step 4: Select board and port

In Arduino IDE:

Select the ESP32 board.
Select the correct COM port.
Install all required libraries.
Step 5: Compile and upload

Click Verify to compile the code.

Then click Upload to flash the firmware to ESP32.

Step 6: Open Serial Monitor

Set baud rate to:

115200

Expected logs:

Connecting WiFi...
WiFi connected!
[MCP] Connected to XiaoZhi server
[MCP] Registered 3 tools
9. XiaoZhi Chatbot Tools

The firmware registers three main tools for XiaoZhi chatbot:

Tool Name	Function
temperature_and_humidity_values	Returns current temperature and humidity
set_temperature_warning	Updates warning temperature threshold
control_alarm	Turns alarm LED and buzzer on or off
10. Test Scenarios
Scenario 1: Read temperature and humidity

User asks:

What is the current temperature?

Expected result:

ESP32 reads DHT11 data.
XiaoZhi replies with current temperature and humidity.
OLED displays the same values.
Scenario 2: Turn on alarm

User asks:

Turn on the alarm.

Expected result:

manualAlarm becomes true.
LED and buzzer start blinking every 300 ms.
XiaoZhi confirms the command.
Scenario 3: Turn off alarm

User asks:

Turn off the alarm.

Expected result:

manualAlarm becomes false.
LED and buzzer turn off if temperature is below the warning threshold.
XiaoZhi confirms the command.
11. Demo Images

12. Project Report

The full project report is available in:

report/nhom2_csudiot_jte.pdf
13. Authors
Nguyen Phi Quoc Bao
Nguyen Duc Anh

Ho Chi Minh City University of Technology and Education

14. License

This project is released for educational and non-commercial research purposes.


GitHub hỗ trợ relative links và image paths trong Markdown, nên đường dẫn như `assets/images/system-block-diagram.png` sẽ hiển thị được ảnh nếu file ảnh tồn tại trong repo. :contentReference[oaicite:5]{index=5}

---

# VII. Viết tài liệu trong thư mục `docs/`

## 1. File `docs/architecture.md`

```markdown
# System Architecture

## 1. Overview

The system follows an AIoT architecture that connects embedded hardware with a cloud-based chatbot interface.

The ESP32 works as the central processing unit. It collects temperature and humidity data from the DHT11 sensor, updates the OLED screen, controls the LED and buzzer, and communicates with XiaoZhi chatbot through Wi-Fi and WebSocket MCP.

## 2. Main Blocks

### 2.1 Input Block

The input block includes the DHT11 sensor. It measures temperature and humidity from the surrounding environment.

### 2.2 Processing Block

The ESP32 receives sensor data, executes control logic, manages Wi-Fi connection, and communicates with XiaoZhi server.

### 2.3 Output Block

The output block includes OLED display, warning LED, and buzzer. The OLED displays environmental data, while LED and buzzer provide visual and audio alarm signals.

### 2.4 Communication Block

The communication block includes Wi-Fi, XiaoZhi cloud server, and chatbot interface. Users can send natural language commands to request data or control the alarm.

Phần này bám sát bài báo: hệ thống được chia thành khối đầu vào, xử lý trung tâm, chấp hành và giao tiếp/giao diện.
