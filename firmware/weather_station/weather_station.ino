/*
 * ================================================================
 *  Solar-Powered IoT Environmental Monitoring Station
 * ================================================================
 *
 *  Description:
 *  ESP8266-based environmental monitoring system integrating:
 *
 *    - BME280  : Temperature, humidity and atmospheric pressure
 *    - MQ-2    : Gas/smoke sensor indication
 *    - INA219  : Voltage, current and power monitoring
 *    - OLED    : Local real-time data visualization
 *    - IR      : Wave-to-wake display interaction
 *    - Wi-Fi   : Wireless connectivity
 *    - ThingSpeak : Cloud telemetry
 *
 *  Target:
 *    NodeMCU 1.0 (ESP-12E Module)
 *    ESP8266 @ 80 MHz
 *
 *  Communication:
 *    I2C  -> BME280, INA219, OLED
 *    Wi-Fi -> ThingSpeak
 *
 *  Firmware:
 *    Arduino / C++
 *
 * ================================================================
 *  IMPORTANT:
 *  Do not commit real Wi-Fi credentials or ThingSpeak API keys
 *  to a public repository.
 * ================================================================
 */

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_BME280.h>
#include <Adafruit_INA219.h>
#include <U8g2lib.h>
#include "ThingSpeak.h"

// ── Board package URL (paste in Arduino → Preferences → Additional Boards) ──
// http://arduino.esp8266.com/stable/package_esp8266com_index.json
// Board: "NodeMCU 1.0 (ESP-12E Module)" · CPU: 80 MHz · Flash: 4MB

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_BME280.h>
#include <Adafruit_INA219.h>
#include <U8g2lib.h>          // Works for both SSD1306 & SH1106
#include "ThingSpeak.h"       // Cloud push

// ================================================================
//  Network & Cloud Configuration
// ================================================================
//
//  IMPORTANT:
//  Keep real credentials private. Do not commit passwords,
//  API keys, or other secrets to a public repository.
//

const char* WIFI_SSID     = "YOUR_SSID";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";

const unsigned long THINGSPEAK_CHANNEL_ID = 0;
const char* THINGSPEAK_WRITE_KEY = "YOUR_WRITE_API_KEY";

// ================================================================
//  Hardware Pin Configuration
// ================================================================

constexpr uint8_t PIN_SCL = D1;      // GPIO5  - I2C clock
constexpr uint8_t PIN_SDA = D2;      // GPIO4  - I2C data
constexpr uint8_t PIN_MQ2 = A0;      // ADC    - MQ-2 analog output
constexpr uint8_t PIN_IR  = D6;      // GPIO12 - IR sensor output
constexpr uint8_t PIN_LED = D7;      // GPIO13 - status LED

// ─────────────────────────────────────────
//  I2C addresses
// ─────────────────────────────────────────
// BME280  : 0x76 (SDO to GND) or 0x77 (SDO to VCC)
// INA219  : 0x40 (A0 A1 both to GND – default)
// OLED    : 0x3C (most 1.3" modules)

// ─────────────────────────────────────────
//  Objects
// ─────────────────────────────────────────
Adafruit_BME280 bme;
Adafruit_INA219 ina219;

// U8g2 for 1.3" SH1106 OLED (I2C, 128×64)
// If your OLED uses SSD1306, replace SH1106 with SSD1306 in the constructor
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

WiFiClient client;

// ================================================================
//  Peripheral Status
// ================================================================

bool bme280Available = false;
bool ina219Available = false;
bool oledAvailable  = false;

// ================================================================
//  Timing Configuration
// ================================================================

constexpr unsigned long CLOUD_UPDATE_INTERVAL_MS = 30000UL;
constexpr unsigned long DISPLAY_TIMEOUT_MS       = 15000UL;
constexpr unsigned long LOOP_DELAY_MS            = 500UL;

unsigned long lastCloudUpdate = 0;
unsigned long displayTimeout  = 0;
// ─────────────────────────────────────────
//  Sensor data
// ─────────────────────────────────────────
float temperature = 0, humidity = 0, pressure = 0;
float busVoltage = 0, current_mA = 0, power_mW = 0;
int   gasRaw = 0;
float gasVoltage = 0;

// ─────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== Weather Station Boot ===");

  // GPIO
  pinMode(PIN_IR,  INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // I2C bus
  Wire.begin(PIN_SDA, PIN_SCL);

 // ================================================================
//  BME280 Initialization
// ================================================================

if (bme.begin(0x76)) {

  bme280Available = true;

  bme.setSampling(
    Adafruit_BME280::MODE_FORCED,
    Adafruit_BME280::SAMPLING_X1,  // Temperature
    Adafruit_BME280::SAMPLING_X1,  // Pressure
    Adafruit_BME280::SAMPLING_X1,  // Humidity
    Adafruit_BME280::FILTER_OFF,
    Adafruit_BME280::STANDBY_MS_1000
  );

  Serial.println("[OK] BME280 initialized");

} else {

  Serial.println("[ERROR] BME280 not detected");
  Serial.println("[INFO] Check wiring and I2C address (0x76/0x77)");
}

  // ================================================================
//  INA219 Power Monitor Initialization
// ================================================================

if (ina219.begin()) {

  ina219Available = true;

  ina219.setCalibration_32V_2A();

  Serial.println("[OK] INA219 initialized");

} else {

  Serial.println("[ERROR] INA219 not detected");
  Serial.println("[INFO] Check I2C wiring and address");
}

 // ================================================================
//  OLED Initialization
// ================================================================

u8g2.begin();
oledAvailable = true;

u8g2.setFont(u8g2_font_6x10_tf);

showSplash();

Serial.println("[OK] OLED initialized");
  // ThingSpeak
  ThingSpeak.begin(client);

 Serial.println();
Serial.println("========================================");
Serial.println("       SYSTEM INITIALIZATION");
Serial.println("========================================");

Serial.printf("BME280 : %s\n", bme280Available ? "OK" : "FAIL");
Serial.printf("INA219 : %s\n", ina219Available ? "OK" : "FAIL");
Serial.printf("OLED   : %s\n", oledAvailable  ? "OK" : "FAIL");
Serial.printf("WiFi   : %s\n", WiFi.status() == WL_CONNECTED ? "OK" : "FAIL");

Serial.println("========================================");
Serial.println("[SYSTEM] Initialization complete");
}

// ─────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // ── IR wave-to-wake ──────────────────
  if (digitalRead(PIN_IR) == LOW) {       // Active-LOW: object/hand detected
    if (!displayOn) {
      Serial.println("[IR] Motion detected → display ON");
      digitalWrite(PIN_LED, HIGH);
    }
    displayOn = true;
    displayTimeout = now + DISPLAY_ON_MS;
  }

  if (displayOn && now > displayTimeout) {
    displayOn = false;
    digitalWrite(PIN_LED, LOW);
    u8g2.clearDisplay();                  // blank OLED to save power
    u8g2.sendBuffer();
    Serial.println("[IR] Display timeout → OFF");
  }

  // ── Read sensors ─────────────────────
  readBME280();
  readINA219();
  readMQ2();

  // ── Update OLED (when awake) ──────────
 if (displayOn && oledAvailable) {
  void updateDisplay() {

  if (!oledAvailable) {
    return;
  }

  char buf[24];
}

  // ── Cloud push every 30 s ─────────────
  if (now - lastPushTime >= PUSH_INTERVAL_MS) {
    lastPushTime = now;

    // Reconnect if dropped
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WiFi] Lost – reconnecting...");
      connectWiFi();
    }

    pushToCloud();
  }

  delay(500); // 0.5 s inner loop keeps display responsive
}

// ─────────────────────────────────────────
//  Sensor readers
// ─────────────────────────────────────────
void readBME280() {

  if (!bme280Available) {
    return;
  }

  bme.takeForcedMeasurement();

  temperature = bme.readTemperature();
  humidity    = bme.readHumidity();
  pressure    = bme.readPressure() / 100.0F;
}

void readINA219() {

  if (!ina219Available) {
    return;
  }

  busVoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW   = ina219.getPower_mW();
}

void readMQ2() {
  gasRaw     = analogRead(PIN_MQ2);      // 0–1023 (ESP8266 ADC = 0–1 V)
  gasVoltage = gasRaw * (1.0 / 1023.0);  // normalise to 0–1 V
  // NOTE: MQ-2 heater needs ~60 s warm-up after power-on for stable readings
}

// ─────────────────────────────────────────
//  OLED display
// ─────────────────────────────────────────
void showSplash() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_8x13B_tf);
  u8g2.drawStr(10, 24, "Weather Station");
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(18, 40, "ESP8266 + Solar");
  u8g2.drawStr(28, 52, "Booting...");
  u8g2.sendBuffer();
  delay(1500);
}

void updateDisplay() {
  char buf[24];
  u8g2.clearBuffer();

  // ── Row 0: header ──
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 9, "WiFi Weather Station");
  u8g2.drawHLine(0, 11, 128);

  // ── Row 1: Temperature & Humidity ──
  snprintf(buf, sizeof(buf), "T:%.1fC  H:%.0f%%", temperature, humidity);
  u8g2.drawStr(0, 22, buf);

  // ── Row 2: Pressure ──
  snprintf(buf, sizeof(buf), "P: %.1f hPa", pressure);
  u8g2.drawStr(0, 33, buf);

  // ── Row 3: Gas ──
  snprintf(buf, sizeof(buf), "Gas: %d (%.2fV)", gasRaw, gasVoltage);
  u8g2.drawStr(0, 44, buf);

  // ── Row 4: Power (solar/battery) ──
  snprintf(buf, sizeof(buf), "V:%.2fV  I:%.0fmA", busVoltage, current_mA);
  u8g2.drawStr(0, 55, buf);

  // ── Row 5: WiFi status (small) ──
  if (WiFi.status() == WL_CONNECTED) {
    snprintf(buf, sizeof(buf), "WiFi OK  %ddBm", WiFi.RSSI());
  } else {
    snprintf(buf, sizeof(buf), "WiFi: connecting");
  }
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(0, 64, buf);

  u8g2.sendBuffer();
}

// ─────────────────────────────────────────
//  Cloud push – ThingSpeak
//  Field mapping:
//    Field 1 = Temperature (°C)
//    Field 2 = Humidity (%RH)
//    Field 3 = Pressure (hPa)
//    Field 4 = Gas ADC raw
//    Field 5 = Bus voltage (V)
//    Field 6 = Current (mA)
//    Field 7 = Power (mW)
//    Field 8 = WiFi RSSI (dBm)
// ─────────────────────────────────────────
void pushToCloud() {
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, pressure);
  ThingSpeak.setField(4, gasRaw);
  ThingSpeak.setField(5, busVoltage);
  ThingSpeak.setField(6, current_mA);
  ThingSpeak.setField(7, power_mW);
  ThingSpeak.setField(8, (float)WiFi.RSSI());

  int httpCode = ThingSpeak.writeFields(CHANNEL_ID, WRITE_KEY);

  if (httpCode == 200) {
    Serial.println("[Cloud] Push OK");
  } else {
    Serial.printf("[Cloud] Error: HTTP %d\n", httpCode);
  }
}

// ─────────────────────────────────────────
//  WiFi helper
// ─────────────────────────────────────────
void connectWiFi() {
  Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WiFi] Connected · IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n[WiFi] FAILED – will retry on next push cycle");
  }
}
