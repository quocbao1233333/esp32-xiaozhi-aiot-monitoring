#pragma once

// =========================
// WiFi Configuration
// =========================
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// =========================
// XiaoZhi MCP Configuration
// =========================
#define XIAOZHI_MCP_ENDPOINT "wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN_HERE"

// =========================
// OLED SPI Pin Configuration
// =========================
#define OLED_MOSI_PIN 19
#define OLED_CLK_PIN 18
#define OLED_DC_PIN 21
#define OLED_CS_PIN 22
#define OLED_RESET_PIN 17

// =========================
// Sensor and Alarm Pins
// =========================
#define DHT_SENSOR_PIN 23
#define WARNING_LED_PIN 5
#define BUZZER_OUTPUT_PIN 4

// =========================
// System Parameters
// =========================
#define DEFAULT_WARNING_TEMP 35.0
#define SENSOR_READ_INTERVAL_MS 2000
#define ALARM_BLINK_INTERVAL_MS 300
