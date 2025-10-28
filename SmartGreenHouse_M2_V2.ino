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

// ТОЛЬКО GlobalInstances.h - он включит все остальное
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
  
  // Инициализация EEPROM и загрузка настроек
  eepromManager.begin();
  if (!eepromManager.loadSettings(systemSettings)) {
    Serial.println("Using default settings");
    strcpy(systemSettings.wifiSSID, "PATT");
    strcpy(systemSettings.wifiPassword, "89396A1F61");
  }
  
  // Инициализация дисплея
  displayManager.begin();
  
  // Инициализация устройств
  deviceManager.begin();
  
  // Подключение к WiFi
  setupWiFi();
  
  // Инициализация веб-сервера
  webInterface.begin(server);
  
  Serial.println("SYSTEM INITIALIZATION COMPLETE");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("IP: " + WiFi.localIP().toString());
  }
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
    
    // Логирование данных
    Serial.println("SYSTEM STATUS");
    Serial.printf("Air: %.1fC %.1f%%\n", sensorData.airTemperature, sensorData.airHumidity);
    Serial.printf("Soil: %.1fC %.1f%%\n", sensorData.soilTemperature, sensorData.soilMoisture);
    Serial.printf("Light: %.0f lux\n", sensorData.lightLevel);
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
  Serial.print("Connecting to ");
  Serial.println(systemSettings.wifiSSID);
  
  WiFi.begin(systemSettings.wifiSSID, systemSettings.wifiPassword);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nStarting AP mode...");
    WiFi.softAP("SmartGreenhouse-M2", "12345678");
    Serial.println("AP IP: " + WiFi.softAPIP().toString());
  }
}