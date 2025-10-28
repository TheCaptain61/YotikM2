#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <TM1637.h>
#include <FastLED.h>
#include <ESP32Servo.h>
#include "Config.h"

class DeviceManager {
public:
    DeviceManager();
    bool begin();
    
    // Чтение датчиков
    void readAllSensors();
    void readBME280();
    void readBH1750();
    void readSoilSensors();
    
    // Управление устройствами
    void setHeater(bool state);
    void setLight(bool state);
    void setVentilation(bool state);
    void setWaterPump(bool state);
    void setServo(int angle);
    void updateDisplay(float temperature, float humidity);
    void setLedColor(uint8_t r, uint8_t g, uint8_t b);
    
    // Получение данных
    SensorData getSensorData() const { return sensorData; }
    DeviceHealth getDeviceHealth() const { return deviceHealth; }
    bool isSystemHealthy() const;
    
    // Проверка состояния устройств
    void checkDeviceHealth();

private:
    SensorData sensorData;
    DeviceHealth deviceHealth;
    PinConfig pins;
    
    // Объекты устройств
    Adafruit_BME280 bme;
    BH1750 lightMeter;
    TM1637 display;
    CRGB leds[16];
    Servo windowServo;
    
    // Вспомогательные методы
    bool initializeBME280();
    bool initializeBH1750();
    bool initializeDisplay();
    bool initializeLedStrip();
    bool initializeServo();
    float readSoilTemperature();
    float readSoilMoisture();
};

#endif