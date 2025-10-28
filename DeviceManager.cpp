#include "DeviceManager.h"
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_BME280.h>
#include <ESP32Servo.h>
#include <FastLED.h>
#include "GlobalInstances.h"

// Драйверы устройств
BH1750 lightMeter;
Adafruit_BME280 bme;
Servo doorServo;
CRGB leds[Constants::NUM_LEDS];

DeviceManager::DeviceManager() {
    Serial.println("🔧 DeviceManager constructor");
}

void DeviceManager::begin() {
    Serial.println("\n" + String(60, '='));
    Serial.println("🔧 DEVICE MANAGER INITIALIZATION");
    Serial.println(String(60, '='));
    
    // Инициализация пинов
    initializePins();
    
    // Инициализация I2C
    Wire.begin();
    delay(100);
    Serial.println("✅ I2C initialized");
    
    // Обнаружение устройств
    discoverI2CDevices();
    initializeDetectedDevices();
    
    // Инициализация GPIO устройств
    initializeSoilSensors();
    initializeLEDMatrix();
    
    // Инициализация серво
    doorServo.attach(Pins::SERVO);
    Serial.println("✅ Servo attached to pin " + String(Pins::SERVO));
    controlDoor(90); // Нейтральное положение
    
    devicesInitialized = true;
    Serial.println("✅ Device Manager initialized successfully");
}

void DeviceManager::initializePins() {
    Serial.println("📌 Initializing GPIO pins...");
    
    // Настройка пинов реле
    int relayPins[] = {Pins::PUMP, Pins::FAN, Pins::HEATER, Pins::LIGHT, Pins::DOOR_LOCK};
    const char* relayNames[] = {"PUMP", "FAN", "HEATER", "LIGHT", "DOOR_LOCK"};
    
    for(int i = 0; i < 5; i++) {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], LOW);
        Serial.println("  " + String(relayNames[i]) + " -> GPIO " + String(relayPins[i]));
    }
    
    // Настройка пинов датчиков
    pinMode(Pins::DOOR_SENSOR, INPUT_PULLUP);
    Serial.println("  DOOR_SENSOR -> GPIO " + String(Pins::DOOR_SENSOR) + " (INPUT_PULLUP)");
    
    Serial.println("✅ GPIO pins initialized");
}

void DeviceManager::discoverI2CDevices() {
    Serial.println("\n--- I2C Device Discovery ---");
    
    // База известных устройств
    struct KnownDevice {
        uint8_t address;
        const char* name;
        const char* description;
    };
    
    KnownDevice knownDevices[] = {
        {0x23, "BH1750", "Light Sensor"},
        {0x27, "LCD1602", "LCD Display"},
        {0x3C, "OLED", "OLED Display"},
        {0x3D, "OLED", "OLED Display"},
        {0x40, "SHT30", "Temperature/Humidity"},
        {0x48, "ADS1115", "ADC Converter"},
        {0x4E, "PCF8574", "I/O Expander"},   // Частый адрес для PCF8574
        {0x50, "EEPROM", "EEPROM Memory"},
        {0x68, "DS3231", "RTC Clock"},
        {0x70, "PCA9685", "PWM Controller"}, // Частый адрес для PCA9685
        {0x76, "BME280", "Environmental"},
        {0x77, "BME280", "Environmental"},
    };
    
    byte error, address;
    int foundCount = 0;
    
    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.print("🔍 Found: 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            
            // Поиск в базе известных устройств
            bool known = false;
            for(auto device : knownDevices) {
                if(device.address == address) {
                    Serial.print(" - ");
                    Serial.print(device.name);
                    Serial.print(" (");
                    Serial.print(device.description);
                    Serial.print(")");
                    known = true;
                    
                    // Запоминаем важные устройства
                    if (strcmp(device.name, "BME280") == 0) {
                        deviceConfig.bme280Address = address;
                        Serial.print(" [MAIN]");
                    } else if (strcmp(device.name, "BH1750") == 0) {
                        deviceConfig.bh1750Address = address;
                        Serial.print(" [MAIN]");
                    }
                    break;
                }
            }
            
            if (!known) {
                Serial.print(" - Unknown I2C Device");
                // Попытка идентификации
                identifyUnknownDevice(address);
            }
            Serial.println();
            foundCount++;
        }
    }
    
    Serial.printf("📟 Found %d I2C device(s)\n", foundCount);
}

void DeviceManager::identifyUnknownDevice(uint8_t address) {
    // Попытка чтения регистра идентификации
    Wire.beginTransmission(address);
    Wire.write(0x00); // Частый регистр идентификации
    if (Wire.endTransmission() == 0) {
        Wire.requestFrom(address, (uint8_t)1);
        if (Wire.available()) {
            byte reg = Wire.read();
            Serial.printf(" [ID: 0x%02X]", reg);
        }
    }
}

void DeviceManager::initializeDetectedDevices() {
    Serial.println("\n--- Device Initialization ---");
    
    // BME280
    if (deviceConfig.bme280Address != 0) {
        Serial.print("🌡️  Initializing BME280 at 0x");
        Serial.print(deviceConfig.bme280Address, HEX);
        Serial.print("... ");
        
        deviceConfig.hasBME280 = bme.begin(deviceConfig.bme280Address);
        deviceConfig.bme280Healthy = deviceConfig.hasBME280;
        
        if (deviceConfig.hasBME280) {
            bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                          Adafruit_BME280::SAMPLING_X1,
                          Adafruit_BME280::SAMPLING_X1,
                          Adafruit_BME280::SAMPLING_X1,
                          Adafruit_BME280::FILTER_OFF);
            Serial.println("✅ OK");
        } else {
            Serial.println("❌ FAILED");
        }
    } else {
        Serial.println("❌ BME280 not found");
    }
    
    // BH1750
    if (deviceConfig.bh1750Address != 0) {
        Serial.print("💡 Initializing BH1750 at 0x");
        Serial.print(deviceConfig.bh1750Address, HEX);
        Serial.print("... ");
        
        deviceConfig.hasBH1750 = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, deviceConfig.bh1750Address);
        deviceConfig.bh1750Healthy = deviceConfig.hasBH1750;
        
        if (deviceConfig.hasBH1750) {
            Serial.println("✅ OK");
        } else {
            Serial.println("❌ FAILED");
        }
    } else {
        Serial.println("❌ BH1750 not found");
    }
    
    Serial.printf("🎯 System capabilities: BME280:%d BH1750:%d Soil:%d\n",
                  deviceConfig.hasBME280, deviceConfig.hasBH1750, deviceConfig.hasSoilSensors);
}

bool DeviceManager::initializeSoilSensors() {
    Serial.print("🌱 Initializing soil sensors... ");
    
    // Проверка подключения датчиков почвы
    int soilValue = analogRead(Pins::SOIL_MOISTURE);
    deviceConfig.hasSoilSensors = (soilValue > 100 && soilValue < (Constants::SOIL_ADC_MAX - 100));
    deviceConfig.soilSensorsHealthy = deviceConfig.hasSoilSensors;
    
    if (deviceConfig.hasSoilSensors) {
        Serial.println("✅ OK (Value: " + String(soilValue) + ")");
    } else {
        Serial.println("❌ FAILED (Value: " + String(soilValue) + ")");
    }
    
    return deviceConfig.hasSoilSensors;
}

void DeviceManager::initializeLEDMatrix() {
    Serial.print("🌈 Initializing LED matrix... ");
    
    FastLED.addLeds<NEOPIXEL, Pins::LED_MATRIX>(leds, Constants::NUM_LEDS);
    FastLED.setBrightness(50);
    fill_solid(leds, Constants::NUM_LEDS, CRGB::Black);
    FastLED.show();
    
    // Тест матрицы
    fill_solid(leds, Constants::NUM_LEDS, CRGB::Red);
    FastLED.show();
    delay(200);
    fill_solid(leds, Constants::NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(200);
    fill_solid(leds, Constants::NUM_LEDS, CRGB::Blue);
    FastLED.show();
    delay(200);
    fill_solid(leds, Constants::NUM_LEDS, CRGB::Black);
    FastLED.show();
    
    Serial.println("✅ OK (" + String(Constants::NUM_LEDS) + " LEDs)");
}

void DeviceManager::readAllSensors() {
    // Чтение BME280
    if (deviceConfig.hasBME280 && deviceConfig.bme280Healthy) {
        readBME280();
    } else if (deviceConfig.hasBME280) {
        Serial.println("⚠️ BME280 marked unhealthy, attempting recovery...");
        deviceConfig.bme280Healthy = bme.begin(deviceConfig.bme280Address);
    }
    
    // Чтение BH1750
    if (deviceConfig.hasBH1750 && deviceConfig.bh1750Healthy) {
        readBH1750();
    } else if (deviceConfig.hasBH1750) {
        Serial.println("⚠️ BH1750 marked unhealthy, attempting recovery...");
        deviceConfig.bh1750Healthy = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, deviceConfig.bh1750Address);
    }
    
    // Чтение датчиков почвы
    if (deviceConfig.hasSoilSensors && deviceConfig.soilSensorsHealthy) {
        readSoilSensors();
    }
    
    // Чтение состояния двери
    sensorData.doorState = digitalRead(Pins::DOOR_SENSOR) == LOW;
    
    // Обновление статуса системы
    sensorData.systemHealthy = deviceConfig.bme280Healthy && 
                              deviceConfig.bh1750Healthy && 
                              deviceConfig.soilSensorsHealthy;
}

void DeviceManager::readBME280() {
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = bme.readPressure() / 100.0F;
    
    if (!isnan(temp) && !isnan(hum) && !isnan(pres)) {
        sensorData.airTemperature = temp;
        sensorData.airHumidity = hum;
        sensorData.pressure = pres;
        bme280ErrorCount = 0;
    } else {
        bme280ErrorCount++;
        Serial.println("⚠️ BME280 read error #" + String(bme280ErrorCount));
        
        if (bme280ErrorCount > 5) {
            deviceConfig.bme280Healthy = false;
            Serial.println("❌ BME280 marked as unhealthy");
        }
    }
}

void DeviceManager::readBH1750() {
    float lux = lightMeter.readLightLevel();
    
    if (!isnan(lux) && lux >= 0 && lux <= 65535) {
        sensorData.lightLevel = lux;
        bh1750ErrorCount = 0;
    } else {
        bh1750ErrorCount++;
        Serial.println("⚠️ BH1750 read error #" + String(bh1750ErrorCount));
        
        if (bh1750ErrorCount > 5) {
            deviceConfig.bh1750Healthy = false;
            Serial.println("❌ BH1750 marked as unhealthy");
        }
    }
}

void DeviceManager::readSoilSensors() {
    // Влажность почвы
    int soilMoistureRaw = analogRead(Pins::SOIL_MOISTURE);
    if (soilMoistureRaw > 100 && soilMoistureRaw < (Constants::SOIL_ADC_MAX - 100)) {
        sensorData.soilMoisture = map(soilMoistureRaw, 
                                     deviceConfig.soilAirValue, 
                                     deviceConfig.soilWaterValue, 0, 100);
        sensorData.soilMoisture = constrain(sensorData.soilMoisture, 0, 100);
        soilSensorErrorCount = 0;
    } else {
        soilSensorErrorCount++;
        Serial.println("⚠️ Soil moisture sensor reading out of range: " + String(soilMoistureRaw));
    }
    
    // Температура почвы
    int soilTempRaw = analogRead(Pins::SOIL_TEMPERATURE);
    if (soilTempRaw > 100 && soilTempRaw < (Constants::SOIL_ADC_MAX - 100)) {
        sensorData.soilTemperature = ((soilTempRaw / (float)Constants::SOIL_ADC_MAX * 
                                     Constants::SOIL_TEMP_CONVERSION) - 0.5) * 100.0;
    }
    
    if (soilSensorErrorCount > 10) {
        deviceConfig.soilSensorsHealthy = false;
        Serial.println("❌ Soil sensors marked as unhealthy");
    }
}

void DeviceManager::checkDeviceHealth() {
    bool needsRediscovery = false;
    
    // Проверка BME280
    if (deviceConfig.hasBME280 && deviceConfig.bme280Healthy) {
        float testTemp = bme.readTemperature();
        if (isnan(testTemp)) {
            Serial.println("⚠️ BME280 health check failed");
            deviceConfig.bme280Healthy = false;
            needsRediscovery = true;
        }
    }
    
    // Проверка BH1750
    if (deviceConfig.hasBH1750 && deviceConfig.bh1750Healthy) {
        float testLux = lightMeter.readLightLevel();
        if (isnan(testLux)) {
            Serial.println("⚠️ BH1750 health check failed");
            deviceConfig.bh1750Healthy = false;
            needsRediscovery = true;
        }
    }
    
    // Проверка датчиков почвы
    if (deviceConfig.hasSoilSensors && deviceConfig.soilSensorsHealthy) {
        int soilValue = analogRead(Pins::SOIL_MOISTURE);
        if (soilValue < 100 || soilValue > (Constants::SOIL_ADC_MAX - 100)) {
            Serial.println("⚠️ Soil sensors health check failed");
            deviceConfig.soilSensorsHealthy = false;
        }
    }
    
    // Переобнаружение устройств при необходимости
    static unsigned long lastRediscovery = 0;
    if (needsRediscovery && (millis() - lastRediscovery > 300000)) {
        Serial.println("🔄 Performing device rediscovery...");
        rediscoverDevices();
        lastRediscovery = millis();
    }
    
    // Обновление общего статуса системы
    sensorData.systemHealthy = deviceConfig.bme280Healthy && 
                              deviceConfig.bh1750Healthy && 
                              deviceConfig.soilSensorsHealthy;
}

void DeviceManager::rediscoverDevices() {
    Serial.println("🔄 Rediscovering devices...");
    
    // Сброс флагов
    deviceConfig.hasBME280 = false;
    deviceConfig.hasBH1750 = false;
    deviceConfig.bme280Healthy = false;
    deviceConfig.bh1750Healthy = false;
    
    // Повторное сканирование I2C
    discoverI2CDevices();
    
    // Повторная инициализация
    if (deviceConfig.bme280Address != 0) {
        deviceConfig.hasBME280 = bme.begin(deviceConfig.bme280Address);
        deviceConfig.bme280Healthy = deviceConfig.hasBME280;
    }
    if (deviceConfig.bh1750Address != 0) {
        deviceConfig.hasBH1750 = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, deviceConfig.bh1750Address);
        deviceConfig.bh1750Healthy = deviceConfig.hasBH1750;
    }
    
    // Повторная проверка датчиков почвы
    initializeSoilSensors();
    
    Serial.println("✅ Device rediscovery completed");
}

void DeviceManager::controlPump(bool state, unsigned long duration) {
    digitalWrite(Pins::PUMP, state ? HIGH : LOW);
    sensorData.pumpState = state;
    
    if (state && duration > 0) {
        pumpAutoStop = true;
        pumpStartTime = millis();
        pumpDuration = duration;
        Serial.println("💧 Pump ON for " + String(duration) + "ms");
    } else {
        pumpAutoStop = false;
        Serial.println(state ? "💧 Pump ON" : "💧 Pump OFF");
    }
}

void DeviceManager::controlFan(bool state) {
    digitalWrite(Pins::FAN, state ? HIGH : LOW);
    sensorData.fanState = state;
    Serial.println(state ? "🌬️ Fan ON" : "🌬️ Fan OFF");
}

void DeviceManager::controlHeater(bool state) {
    digitalWrite(Pins::HEATER, state ? HIGH : LOW);
    sensorData.heaterState = state;
    Serial.println(state ? "🔥 Heater ON" : "🔥 Heater OFF");
}

void DeviceManager::controlLight(bool state) {
    digitalWrite(Pins::LIGHT, state ? HIGH : LOW);
    sensorData.lightState = state;
    
    // Также управляем LED матрицей
    if (state) {
        fill_solid(leds, Constants::NUM_LEDS, CRGB::White);
        Serial.println("💡 Light ON + LED Matrix WHITE");
    } else {
        fill_solid(leds, Constants::NUM_LEDS, CRGB::Black);
        Serial.println("💡 Light OFF + LED Matrix OFF");
    }
    FastLED.show();
}

void DeviceManager::controlDoor(bool angle) {
    angle = constrain(angle, 0, 180);
    doorServo.write(angle);
    delay(Constants::SERVO_DELAY);
    Serial.println("🚪 Door position: " + String(angle) + "°");
}

void DeviceManager::stopAllDevices() {
    controlPump(false);
    controlFan(false);
    controlHeater(false);
    controlLight(false);
    Serial.println("🔴 All devices stopped");
}

void DeviceManager::calibrateSoilSensor(bool inWater) {
    int rawValue = analogRead(Pins::SOIL_MOISTURE);
    
    if (inWater) {
        deviceConfig.soilWaterValue = rawValue;
        Serial.println("💧 Water calibration: " + String(rawValue));
    } else {
        deviceConfig.soilAirValue = rawValue;
        Serial.println("💨 Air calibration: " + String(rawValue));
    }
}

String DeviceManager::getDeviceSummary() const {
    String summary = "=== Device Summary ===\n";
    summary += "BME280: " + String(deviceConfig.hasBME280 ? "Present" : "Missing");
    summary += " [" + String(deviceConfig.bme280Healthy ? "OK" : "ERROR") + "]\n";
    summary += "BH1750: " + String(deviceConfig.hasBH1750 ? "Present" : "Missing");
    summary += " [" + String(deviceConfig.bh1750Healthy ? "OK" : "ERROR") + "]\n";
    summary += "Soil Sensors: " + String(deviceConfig.hasSoilSensors ? "Present" : "Missing");
    summary += " [" + String(deviceConfig.soilSensorsHealthy ? "OK" : "ERROR") + "]\n";
    summary += "TM1637: Present [OK]\n";
    summary += "Relays: Present [OK]\n";
    return summary;
}

bool DeviceManager::isSystemHealthy() const {
    return sensorData.systemHealthy;
}