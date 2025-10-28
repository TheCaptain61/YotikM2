#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Версия конфигурации для миграции EEPROM
#define CONFIG_VERSION 3
#define EEPROM_SIZE 512

// ===== Структуры для хранения данных =====
struct SystemSettings {
  uint8_t version = CONFIG_VERSION;
  char wifiSSID[32] = "";
  char wifiPassword[32] = "";
  float tempSetpoint = 25.0;
  float humSetpoint = 60.0;
  float soilMoistureSetpoint = 50.0;
  int lightOnHour = 8;
  int lightOffHour = 20;
  bool automationEnabled = true;
  uint8_t displayBrightness = 7;
  bool use24HourFormat = true;
};

struct SensorData {
  float airTemperature = NAN;
  float airHumidity = NAN;
  float soilMoisture = NAN;
  float soilTemperature = NAN;
  float lightLevel = NAN;
  
  bool pumpState = false;
  bool fanState = false;
  bool lightState = false;  // УДАЛЕН heaterState
  bool doorState = false;
};

struct DeviceConfig {
  // Адреса I2C устройств
  uint8_t bme280Address = 0;
  uint8_t bh1750Address = 0;
  
  // Флаги наличия устройств
  bool hasBME280 = false;
  bool hasBH1750 = false;
  bool hasTM1637 = true;
  bool hasSoilSensors = false;
  bool hasRelays = true;
  
  // Калибровочные значения
  float soilAirValue = 2800.0;
  float soilWaterValue = 1200.0;
  
  // Статусы устройств
  bool bme280Healthy = false;
  bool bh1750Healthy = false;
  bool soilSensorsHealthy = false;
};

// ===== Конфигурация пинов для ESP32 =====
namespace Pins {
  // Управление
  constexpr uint8_t PUMP = 17;
  constexpr uint8_t FAN = 16;
  constexpr uint8_t LIGHT = 5;
  constexpr uint8_t DOOR_LOCK = 15;
  
  // Датчики
  constexpr uint8_t SOIL_MOISTURE = 34;
  constexpr uint8_t SOIL_TEMPERATURE = 35;
  constexpr uint8_t DOOR_SENSOR = 12;
  
  // Дисплей
  constexpr uint8_t TM1637_CLK = 13;
  constexpr uint8_t TM1637_DIO = 14;
  
  // LED матрица
  constexpr uint8_t LED_MATRIX = 18;
  
  // Сервомотор
  constexpr uint8_t SERVO = 19;
}

// ===== Константы системы =====
namespace Constants {
  constexpr uint16_t NUM_LEDS = 64;
  constexpr uint16_t SOIL_ADC_MAX = 4095;
  constexpr float SOIL_TEMP_CONVERSION = 6.27;
  constexpr unsigned long SERVO_DELAY = 1000;
  constexpr unsigned long PUMP_DURATION = 5000;
}

// ===== Глобальные экземпляры =====
extern SystemSettings systemSettings;
extern SensorData sensorData;
extern DeviceConfig deviceConfig;

#endif