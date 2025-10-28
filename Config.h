#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Структура для хранения данных с датчиков
struct SensorData {
    float airTemperature = 0.0;
    float airHumidity = 0.0;
    float pressure = 0.0;
    float lightLevel = 0.0;
    float soilMoisture = 0.0;
    float soilTemperature = 0.0;
    bool heaterState = false;
    bool lightState = false;
    bool ventilationState = false;
    bool waterPumpState = false;
    bool systemHealthy = false;
    String errorMessage = "";
    unsigned long lastUpdate = 0;
};

// Структура для хранения настроек устройства
struct DeviceSettings {
    // Температура
    float tempSetpoint = 25.0;
    float tempHysteresis = 1.0;
    
    // Освещение
    float lightThreshold = 100.0;
    
    // Вентиляция
    float ventilationTemp = 30.0;
    float ventilationHumidity = 80.0;
    
    // Полив
    float soilMoistureThreshold = 40.0;
    unsigned long wateringDuration = 5000;
    
    // Сеть
    String wifiSSID = "";
    String wifiPassword = "";
    String deviceName = "SmartGreenhouse";
    
    // Интервалы обновления
    unsigned long sensorReadInterval = 5000;
    unsigned long controlUpdateInterval = 2000;
};

// Конфигурация пинов
struct PinConfig {
    // Датчики
    int bme280SDA = 21;
    int bme280SCL = 22;
    int bh1750SDA = 21;
    int bh1750SCL = 22;
    int soilMoisturePin = 34;
    int soilTempPin = 35;
    
    // Исполнительные устройства
    int heaterPin = 4;
    int lightPin = 5;
    int ventilationPin = 6;
    int waterPumpPin = 7;
    int servoPin = 8;
    
    // Дисплей
    int displayDIO = 13;
    int displayCLK = 14;
    
    // Светодиодная лента
    int ledPin = 11;
    int numLeds = 16;
};

// Конфигурация состояния устройств
struct DeviceHealth {
    bool bme280Healthy = false;
    bool bh1750Healthy = false;
    bool soilSensorHealthy = false;
    bool displayHealthy = false;
    bool ledStripHealthy = false;
    bool lastUpdateSuccess = false;
};

#endif