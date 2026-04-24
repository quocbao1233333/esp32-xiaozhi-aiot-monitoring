#include <WiFi.h>
#include <WebSocketMCP.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ====== CẤU HÌNH CHÂN OLED SPI 7 CHÂN ======
#define OLED_MOSI  19 // Chân D1 trên OLED
#define OLED_CLK   18 // Chân D0 trên OLED
#define OLED_DC    21 // Chân DC trên OLED
#define OLED_CS    22 // Chân CS trên OLED
#define OLED_RESET 17 // Chân RES trên OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// ====== CẢM BIẾN & CẢNH BÁO ======
#define DHTPIN 23     // Chân Data của DHT11 cắm vào D23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define WARNING_LED 5 // Chân đèn LED cắm vào D5
#define BUZZER_PIN 4 //  Chân Còi hú (Buzzer) cắm vào D26

// ====== WiFi ======
const char* ssid = "xxxxxx";
const char* password = "xxxxxx";  

// ====== MCP ======
const char* mcpEndpoint = "wss://api.xiaozhi.me/mcp/?xxxxxxxxx";

WebSocketMCP mcpClient;

// Giá trị cảm biến
float temperature = 0;
float humidity = 0;

// Ngưỡng cảnh báo và Trạng thái
float warningTemp = 35.0; // Đã nâng ngưỡng mặc định lên 35 độ
bool manualAlarm = false; // Biến lưu trạng thái bật/tắt cảnh báo từ xa do AI điều khiển

// Biến thời gian
unsigned long lastSensorRead = 0;
unsigned long lastBlink = 0;
bool ledState = false;

// ========================================================================================
// CALLBACK MCP
// ========================================================================================
void onConnectionStatus(bool connected) {
    if (connected) {
        Serial.println("[MCP] ✅ Đã kết nối tới máy chủ Xiaozhi");
        registerMcpTools();
    } else {
        Serial.println("[MCP] ⚠️ Mất kết nối MCP");
    }
}

// ========================================================================================
// MCP TOOLS (CÔNG CỤ CHO AI)
// ========================================================================================
void registerMcpTools() {
    // 📌 1. Tool báo giá trị cảm biến cho AI
    mcpClient.registerTool(
        "temperature_and_humidity_values",
        "Báo giá trị cảm biến DHT11",
        R"({
            "type": "object",
            "properties": {
                "temperature": { "type": "number" },
                "humidity": { "type": "number" }
            },
            "required": ["temperature", "humidity"]
        })",
        [](const String& args) {
            DynamicJsonDocument doc(256);
            doc["temperature"] = temperature;
            doc["humidity"] = humidity;
            String json;
            serializeJson(doc, json);
            return WebSocketMCP::ToolResponse(json);
        }
    );

    // 📌 2. Tool cho phép AI cài đặt ngưỡng cảnh báo
    mcpClient.registerTool(
        "set_temperature_warning",
        "Cài đặt nhiệt độ cảnh báo",
        R"({
            "type": "object",
            "properties": {
                "max_temperature": { "type": "number" }
            },
            "required": ["max_temperature"]
        })",
        [](const String& args) {
            DynamicJsonDocument doc(256);
            deserializeJson(doc, args);
            warningTemp = doc["max_temperature"].as<float>();
            String json = "{\"success\":true,\"new_limit\":" + String(warningTemp) + "}";
            return WebSocketMCP::ToolResponse(json);
        }
    );

    // Bật/Tắt Còi hú và Đèn cảnh báo từ xa
    mcpClient.registerTool(
        "control_alarm",
        "Bật hoặc tắt đèn cảnh báo và còi hú từ xa",
        R"({
            "type": "object",
            "properties": {
                "state": { 
                    "type": "string",
                    "enum": ["on", "off"]
                }
            },
            "required": ["state"]
        })",
        [](const String& args) {
            DynamicJsonDocument doc(256);
            deserializeJson(doc, args);
            String state = doc["state"].as<String>();

            if (state == "on") {
                manualAlarm = true; // Kích hoạt báo động thủ công
                Serial.println(">>> AI ĐÃ BẬT BÁO ĐỘNG TỪ XA <<<");
            } else {
                manualAlarm = false; // Tắt báo động thủ công
                Serial.println(">>> AI ĐÃ TẮT BÁO ĐỘNG TỪ XA <<<");
            }

            return WebSocketMCP::ToolResponse("{\"success\":true,\"state\":\"" + state + "\"}");
        }
    );
    Serial.println("[MCP] 🛠️ Đã đăng ký 3 Tool: Cảm biến, Ngưỡng nhiệt, và Báo động từ xa");
}

// ========================================================================================
// HIỂN THỊ OLED
// ========================================================================================
void updateOLED() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("Nhiet do: "); display.print(temperature, 1); display.println(" C");

    display.setCursor(0, 16);
    display.print("Do am   : "); display.print(humidity, 1); display.println(" %");

    display.setCursor(0, 32);
    display.print("Canh bao: "); display.print(warningTemp, 1); display.println(" C");

    // Hiển thị cảnh báo trên màn hình nếu nhiệt độ cao HOẶC bị bật từ xa
    if (temperature > warningTemp || manualAlarm) {
        display.setCursor(0, 48);
        display.print("!!! QUA NHIET !!!");
    } else {
        display.setCursor(0, 48);
        display.print("Trang thai OK");
    }

    display.display();
}

// ========================================================================================
// SETUP
// ========================================================================================
void setup() {
    Serial.begin(115200);

    // Khởi tạo Đèn và Còi
    pinMode(WARNING_LED, OUTPUT);
    digitalWrite(WARNING_LED, LOW);
    
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    dht.begin();

    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("OLED ERROR!");
        while (1);
    }
    display.clearDisplay();
    display.display();

    Serial.println("Connecting WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(200);
    }
    Serial.println("\n✅ WiFi connected!");
    
    mcpClient.begin(mcpEndpoint, onConnectionStatus);
}

// ========================================================================================
// LOOP
// ========================================================================================
void loop() {
    mcpClient.loop();

    unsigned long now = millis();

    // 1. Đọc cảm biến mỗi 2 giây (đã tăng lên 2s để DHT11 không bị lỗi "nan")
    if (now - lastSensorRead >= 2000) {
        lastSensorRead = now;
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        updateOLED();
    }

    // LOGIC BÁO ĐỘNG (Bật nếu Nhiệt độ > 35 ĐỘ hoặc AI ra lệnh BẬT)
    bool isAlarming = (temperature > warningTemp) || (manualAlarm == true);

    if (isAlarming) {
        // Còi và Đèn cùng nhấp nháy mỗi 300ms
        if (now - lastBlink >= 300) {
            lastBlink = now;
            ledState = !ledState;
            digitalWrite(WARNING_LED, ledState);
            digitalWrite(BUZZER_PIN, ledState); 
        }
    } else {
        // Trạng thái bình thường: Tắt toàn bộ
        digitalWrite(WARNING_LED, LOW);
        digitalWrite(BUZZER_PIN, LOW);
        ledState = false;
    }
}
