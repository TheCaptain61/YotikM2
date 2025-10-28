/*
 * Умная Теплица ЙоТик М2
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <TM1637Display.h>
#include <FastLED.h>
#include <ESP32Servo.h>

// Config должен быть первым
#include "Config.h"
// Затем GlobalInstances
#include "GlobalInstances.h"

WebServer server(80);

// Глобальные структуры данных
SystemSettings systemSettings;
SensorData sensorData;
DeviceConfig deviceConfig;

// Таймеры
unsigned long previousSensorRead = 0;
unsigned long previousDisplayUpdate = 0;
unsigned long previousHealthCheck = 0;

const unsigned long SENSOR_READ_INTERVAL = 30000;
const unsigned long DISPLAY_UPDATE_INTERVAL = 2000;
const unsigned long HEALTH_CHECK_INTERVAL = 60000;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nSMART GREENHOUSE M2 - INITIALIZATION");
  
  // Тест чисел
  for (int i = 0; i <= 100; i += 20) {
    char buffer[5];
    sprintf(buffer, "%4d", i);
    displayManager.showMessage(buffer);
    delay(300);
  }
  
  displayManager.clear();
  Serial.println("✅ Display test complete");
  
  // Инициализация EEPROM и загрузка настроек
  eepromManager.begin();
  if (!eepromManager.loadSettings(systemSettings)) {
    Serial.println("Using default settings");
    strcpy(systemSettings.wifiSSID, "GreenHouse");
    strcpy(systemSettings.wifiPassword, "password");
  }
  
  // Инициализация устройств
  deviceManager.begin();
  
  // Подключение к WiFi
  setupWiFi();
  
  // Инициализация веб-сервера
  webInterface.begin(server);
  
  Serial.println("✅ SYSTEM INITIALIZATION COMPLETE");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("📡 IP: " + WiFi.localIP().toString());
  }
  
  displayManager.showMessage("DONE");
  delay(1000);
}

void loop() {
  server.handleClient();
  
  unsigned long currentMillis = millis();
  
  // Чтение датчиков
  if (currentMillis - previousSensorRead >= SENSOR_READ_INTERVAL) {
    previousSensorRead = currentMillis;
    deviceManager.readAllSensors();   
    if (systemSettings.automationEnabled) {
      automation.process(sensorData, systemSettings, deviceManager);
    }
  }
  
  // Обновление дисплея
  if (currentMillis - previousDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    previousDisplayUpdate = currentMillis;
    displayManager.updateDisplay(sensorData, systemSettings);
  }
  
  // Проверка здоровья системы
  if (currentMillis - previousHealthCheck >= HEALTH_CHECK_INTERVAL) {
    previousHealthCheck = currentMillis;
    deviceManager.checkDeviceHealth();
  }
}

void setupWiFi() {
  Serial.print("📡 Connecting to ");
  Serial.println(systemSettings.wifiSSID);
  
  WiFi.begin(systemSettings.wifiSSID, systemSettings.wifiPassword);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Connected! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n🔄 Starting AP mode...");
    WiFi.softAP("SmartGreenhouse-M2", "12345678");
    Serial.println("📶 AP IP: " + WiFi.softAPIP().toString());
  }
}