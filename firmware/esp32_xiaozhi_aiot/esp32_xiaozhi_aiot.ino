/*
  Project: ESP32 XiaoZhi AIoT Monitoring and Alarm Control System
  Board  : ESP32
  Sensor : DHT11
  Display: SSD1306 OLED SPI
  Output : Warning LED + Buzzer
  Cloud  : XiaoZhi MCP Server via WebSocket

  Description:
  - Read temperature and humidity from DHT11.
  - Display sensor values on OLED.
  - Register 3 tools for XiaoZhi chatbot:
      1. temperature_and_humidity_values
      2. set_temperature_warning
      3. control_alarm
  - Activate LED and buzzer alarm if:
      temperature > warningTemp
      OR manualAlarm == true
*/

#include <WiFi.h>
#include <WebSocketMCP.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// ============================================================
// OLED CONFIGURATION
// ============================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI  OLED_MOSI_PIN
#define OLED_CLK   OLED_CLK_PIN
#define OLED_DC    OLED_DC_PIN
#define OLED_CS    OLED_CS_PIN
#define OLED_RESET OLED_RESET_PIN

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  OLED_MOSI,
  OLED_CLK,
  OLED_DC,
  OLED_RESET,
  OLED_CS
);

// ============================================================
// SENSOR AND OUTPUT CONFIGURATION
// ============================================================

#define DHTPIN  DHT_SENSOR_PIN
#define DHTTYPE DHT11

#define WARNING_LED WARNING_LED_PIN
#define BUZZER_PIN  BUZZER_OUTPUT_PIN

DHT dht(DHTPIN, DHTTYPE);

// ============================================================
// WIFI AND MCP CONFIGURATION
// ============================================================

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mcpEndpoint = XIAOZHI_MCP_ENDPOINT;

WebSocketMCP mcpClient;

// ============================================================
// GLOBAL VARIABLES
// ============================================================

float temperature = 0.0;
float humidity = 0.0;
float warningTemp = DEFAULT_WARNING_TEMP;

bool manualAlarm = false;
bool ledState = false;

unsigned long lastSensorRead = 0;
unsigned long lastBlink = 0;

// ============================================================
// FUNCTION DECLARATIONS
// ============================================================

void connectWiFi();
void onConnectionStatus(bool connected);
void registerMcpTools();
void readSensorData();
void updateOLED();
void handleAlarm();
void showBootScreen();
void showWiFiScreen();
void showMcpScreen();
String buildSensorJson();
String buildSuccessJson(const String& key, const String& value);
String buildSuccessJsonNumber(const String& key, float value);

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("========================================");
  Serial.println("ESP32 XiaoZhi AIoT System Starting...");
  Serial.println("========================================");

  // Initialize output pins
  pinMode(WARNING_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(WARNING_LED, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Initialize DHT11 sensor
  dht.begin();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("[OLED] ERROR: OLED initialization failed!");
    while (true) {
      digitalWrite(WARNING_LED, HIGH);
      delay(200);
      digitalWrite(WARNING_LED, LOW);
      delay(200);
    }
  }

  display.clearDisplay();
  display.display();
  showBootScreen();

  // Connect WiFi
  connectWiFi();

  // Connect XiaoZhi MCP server
  showMcpScreen();
  mcpClient.begin(mcpEndpoint, onConnectionStatus);

  Serial.println("[SYSTEM] Setup completed.");
}

// ============================================================
// LOOP
// ============================================================

void loop() {
  // Keep MCP/WebSocket connection alive
  mcpClient.loop();

  // Read sensor periodically without blocking the program
  unsigned long now = millis();

  if (now - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
    lastSensorRead = now;
    readSensorData();
    updateOLED();
  }

  // Handle alarm logic continuously
  handleAlarm();
}

// ============================================================
// WIFI CONNECTION
// ============================================================

void connectWiFi() {
  Serial.println("[WiFi] Connecting...");
  showWiFiScreen();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int retryCount = 0;

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
    retryCount++;

    if (retryCount >= 100) {
      Serial.println();
      Serial.println("[WiFi] ERROR: Connection timeout. Restarting ESP32...");
      ESP.restart();
    }
  }

  Serial.println();
  Serial.println("[WiFi] Connected successfully!");
  Serial.print("[WiFi] IP Address: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("WiFi Connected");

  display.setCursor(0, 16);
  display.print("IP:");

  display.setCursor(0, 28);
  display.println(WiFi.localIP());

  display.display();
  delay(1000);
}

// ============================================================
// MCP CALLBACK
// ============================================================

void onConnectionStatus(bool connected) {
  if (connected) {
    Serial.println("[MCP] Connected to XiaoZhi server.");
    registerMcpTools();
  } else {
    Serial.println("[MCP] Disconnected from XiaoZhi server.");
  }
}

// ============================================================
// MCP TOOLS REGISTRATION
// ============================================================

void registerMcpTools() {
  /*
    Tool 1:
    Return current temperature and humidity values to XiaoZhi.
  */
  mcpClient.registerTool(
    "temperature_and_humidity_values",
    "Return current temperature and humidity values from DHT11 sensor",
    R"({
      "type": "object",
      "properties": {
        "temperature": {
          "type": "number",
          "description": "Current temperature in Celsius"
        },
        "humidity": {
          "type": "number",
          "description": "Current humidity in percentage"
        }
      },
      "required": ["temperature", "humidity"]
    })",
    [](const String& args) {
      String json = buildSensorJson();

      Serial.println("[MCP Tool] temperature_and_humidity_values called.");
      Serial.print("[MCP Tool] Response: ");
      Serial.println(json);

      return WebSocketMCP::ToolResponse(json);
    }
  );

  /*
    Tool 2:
    Allow XiaoZhi to update the warning temperature threshold.
  */
  mcpClient.registerTool(
    "set_temperature_warning",
    "Set maximum temperature threshold for alarm warning",
    R"({
      "type": "object",
      "properties": {
        "max_temperature": {
          "type": "number",
          "description": "Maximum temperature threshold in Celsius"
        }
      },
      "required": ["max_temperature"]
    })",
    [](const String& args) {
      Serial.println("[MCP Tool] set_temperature_warning called.");
      Serial.print("[MCP Tool] Args: ");
      Serial.println(args);

      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, args);

      if (error) {
        Serial.println("[MCP Tool] JSON parse error.");
        return WebSocketMCP::ToolResponse(
          "{\"success\":false,\"error\":\"invalid_json\"}"
        );
      }

      if (!doc.containsKey("max_temperature")) {
        Serial.println("[MCP Tool] Missing max_temperature.");
        return WebSocketMCP::ToolResponse(
          "{\"success\":false,\"error\":\"missing_max_temperature\"}"
        );
      }

      float newLimit = doc["max_temperature"].as<float>();

      if (newLimit <= 0 || newLimit > 100) {
        Serial.println("[MCP Tool] Invalid temperature threshold.");
        return WebSocketMCP::ToolResponse(
          "{\"success\":false,\"error\":\"invalid_temperature_range\"}"
        );
      }

      warningTemp = newLimit;
      updateOLED();

      String json = buildSuccessJsonNumber("new_limit", warningTemp);

      Serial.print("[MCP Tool] New warning temperature: ");
      Serial.println(warningTemp);

      return WebSocketMCP::ToolResponse(json);
    }
  );

  /*
    Tool 3:
    Allow XiaoZhi to turn alarm LED and buzzer on or off remotely.
  */
  mcpClient.registerTool(
    "control_alarm",
    "Turn warning LED and buzzer on or off remotely",
    R"({
      "type": "object",
      "properties": {
        "state": {
          "type": "string",
          "enum": ["on", "off"],
          "description": "Alarm control state"
        }
      },
      "required": ["state"]
    })",
    [](const String& args) {
      Serial.println("[MCP Tool] control_alarm called.");
      Serial.print("[MCP Tool] Args: ");
      Serial.println(args);

      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, args);

      if (error) {
        Serial.println("[MCP Tool] JSON parse error.");
        return WebSocketMCP::ToolResponse(
          "{\"success\":false,\"error\":\"invalid_json\"}"
        );
      }

      if (!doc.containsKey("state")) {
        Serial.println("[MCP Tool] Missing state.");
        return WebSocketMCP::ToolResponse(
          "{\"success\":false,\"error\":\"missing_state\"}"
        );
      }

      String state = doc["state"].as<String>();
      state.toLowerCase();

      if (state == "on") {
        manualAlarm = true;
        Serial.println("[ALARM] Manual alarm turned ON by XiaoZhi.");
      } else if (state == "off") {
        manualAlarm = false;
        Serial.println("[ALARM] Manual alarm turned OFF by XiaoZhi.");
      } else {
        Serial.println("[MCP Tool] Invalid state value.");
        return WebSocketMCP::ToolResponse(
          "{\"success\":false,\"error\":\"invalid_state\"}"
        );
      }

      updateOLED();

      String json = buildSuccessJson("state", state);
      return WebSocketMCP::ToolResponse(json);
    }
  );

  Serial.println("[MCP] Registered 3 tools successfully:");
  Serial.println("      1. temperature_and_humidity_values");
  Serial.println("      2. set_temperature_warning");
  Serial.println("      3. control_alarm");
}

// ============================================================
// SENSOR READING
// ============================================================

void readSensorData() {
  float newTemperature = dht.readTemperature();
  float newHumidity = dht.readHumidity();

  if (isnan(newTemperature) || isnan(newHumidity)) {
    Serial.println("[DHT11] ERROR: Failed to read sensor data.");
    return;
  }

  temperature = newTemperature;
  humidity = newHumidity;

  Serial.print("[DHT11] Temperature: ");
  Serial.print(temperature, 1);
  Serial.print(" C | Humidity: ");
  Serial.print(humidity, 1);
  Serial.println(" %");
}

// ============================================================
// OLED DISPLAY
// ============================================================

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("ESP32 XiaoZhi AIoT");

  display.setCursor(0, 14);
  display.print("Nhiet do: ");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 26);
  display.print("Do am   : ");
  display.print(humidity, 1);
  display.println(" %");

  display.setCursor(0, 38);
  display.print("Nguong  : ");
  display.print(warningTemp, 1);
  display.println(" C");

  display.setCursor(0, 52);

  if ((temperature > warningTemp) || manualAlarm) {
    display.print("CANH BAO: ON");
  } else {
    display.print("Trang thai: OK");
  }

  display.display();
}

// ============================================================
// ALARM CONTROL
// ============================================================

void handleAlarm() {
  unsigned long now = millis();

  bool isAlarming = (temperature > warningTemp) || (manualAlarm == true);

  if (isAlarming) {
    if (now - lastBlink >= ALARM_BLINK_INTERVAL_MS) {
      lastBlink = now;
      ledState = !ledState;

      digitalWrite(WARNING_LED, ledState ? HIGH : LOW);
      digitalWrite(BUZZER_PIN, ledState ? HIGH : LOW);
    }
  } else {
    digitalWrite(WARNING_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    ledState = false;
  }
}

// ============================================================
// OLED BOOT SCREENS
// ============================================================

void showBootScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("ESP32 XiaoZhi");

  display.setCursor(0, 16);
  display.println("AIoT Monitoring");

  display.setCursor(0, 32);
  display.println("Initializing...");

  display.display();
  delay(1000);
}

void showWiFiScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Connecting WiFi...");

  display.setCursor(0, 16);
  display.println("Please wait");

  display.display();
}

void showMcpScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Connecting MCP...");

  display.setCursor(0, 16);
  display.println("XiaoZhi Server");

  display.display();
}

// ============================================================
// JSON HELPERS
// ============================================================

String buildSensorJson() {
  DynamicJsonDocument doc(256);

  doc["success"] = true;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["warning_temperature"] = warningTemp;
  doc["manual_alarm"] = manualAlarm;

  String json;
  serializeJson(doc, json);

  return json;
}

String buildSuccessJson(const String& key, const String& value) {
  DynamicJsonDocument doc(256);

  doc["success"] = true;
  doc[key] = value;

  String json;
  serializeJson(doc, json);

  return json;
}

String buildSuccessJsonNumber(const String& key, float value) {
  DynamicJsonDocument doc(256);

  doc["success"] = true;
  doc[key] = value;

  String json;
  serializeJson(doc, json);

  return json;
}

