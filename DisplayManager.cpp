#include "DeviceManager.h"
#include "Config.h"

DeviceManager::DeviceManager() : display(9, 10) {
    // Инициализация конфигурации пинов
    pins = PinConfig();
}

bool DeviceManager::begin() {
    Wire.begin(pins.bme280SDA, pins.bme280SCL);
    
    bool success = true;
    success &= initializeBME280();
    success &= initializeBH1750();
    success &= initializeDisplay();
    success &= initializeLedStrip();
    success &= initializeServo();
    
    // Инициализация пинов управления
    pinMode(pins.heaterPin, OUTPUT);
    pinMode(pins.lightPin, OUTPUT);
    pinMode(pins.ventilationPin, OUTPUT);
    pinMode(pins.waterPumpPin, OUTPUT);
    pinMode(pins.soilMoisturePin, INPUT);
    pinMode(pins.soilTempPin, INPUT);
    
    digitalWrite(pins.heaterPin, LOW);
    digitalWrite(pins.lightPin, LOW);
    digitalWrite(pins.ventilationPin, LOW);
    digitalWrite(pins.waterPumpPin, LOW);
    
    sensorData.systemHealthy = success;
    deviceHealth.lastUpdateSuccess = success;
    
    return success;
}

bool DeviceManager::initializeBME280() {
    deviceHealth.bme280Healthy = bme.begin(0x76, &Wire);
    return deviceHealth.bme280Healthy;
}

bool DeviceManager::initializeBH1750() {
    deviceHealth.bh1750Healthy = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
    return deviceHealth.bh1750Healthy;
}

bool DeviceManager::initializeDisplay() {
    display.init();
    display.set(BRIGHT_TYPICAL);
    deviceHealth.displayHealthy = true;
    return true;
}

bool DeviceManager::initializeLedStrip() {
    FastLED.addLeds<WS2812, 11, GRB>(leds, 16);
    fill_solid(leds, 16, CRGB::Green);
    FastLED.show();
    deviceHealth.ledStripHealthy = true;
    return true;
}

bool DeviceManager::initializeServo() {
    windowServo.setPeriodHertz(50);
    windowServo.attach(pins.servoPin, 500, 2400);
    return true;
}

void DeviceManager::readAllSensors() {
    readBME280();
    readBH1750();
    readSoilSensors();
    
    // Обновление общего состояния системы
    sensorData.systemHealthy = deviceHealth.bme280Healthy && 
                              deviceHealth.bh1750Healthy && 
                              deviceHealth.soilSensorHealthy;
    
    sensorData.lastUpdate = millis();
    deviceHealth.lastUpdateSuccess = true;
}

void DeviceManager::readBME280() {
    if (deviceHealth.bme280Healthy) {
        sensorData.airTemperature = bme.readTemperature();
        sensorData.airHumidity = bme.readHumidity();
        sensorData.pressure = bme.readPressure() / 100.0F;
    }
}

void DeviceManager::readBH1750() {
    if (deviceHealth.bh1750Healthy) {
        sensorData.lightLevel = lightMeter.readLightLevel();
    }
}

void DeviceManager::readSoilSensors() {
    sensorData.soilMoisture = readSoilMoisture();
    sensorData.soilTemperature = readSoilTemperature();
    deviceHealth.soilSensorHealthy = (sensorData.soilMoisture >= 0 && sensorData.soilMoisture <= 100);
}

float DeviceManager::readSoilTemperature() {
    int analogValue = analogRead(pins.soilTempPin);
    float voltage = analogValue * (3.3 / 4095.0);
    return (voltage - 0.5) * 100;
}

float DeviceManager::readSoilMoisture() {
    int analogValue = analogRead(pins.soilMoisturePin);
    return map(analogValue, 0, 4095, 0, 100);
}

void DeviceManager::setHeater(bool state) {
    digitalWrite(pins.heaterPin, state ? HIGH : LOW);
    sensorData.heaterState = state;
}

void DeviceManager::setLight(bool state) {
    digitalWrite(pins.lightPin, state ? HIGH : LOW);
    sensorData.lightState = state;
}

void DeviceManager::setVentilation(bool state) {
    digitalWrite(pins.ventilationPin, state ? HIGH : LOW);
    sensorData.ventilationState = state;
}

void DeviceManager::setWaterPump(bool state) {
    digitalWrite(pins.waterPumpPin, state ? HIGH : LOW);
    sensorData.waterPumpState = state;
}

void DeviceManager::setServo(int angle) {
    windowServo.write(constrain(angle, 0, 180));
}

void DeviceManager::updateDisplay(float temperature, float humidity) {
    if (deviceHealth.displayHealthy) {
        display.display(static_cast<int>(temperature));
        // TM1637 может отображать только числа, поэтому чередуем отображение
        static unsigned long lastDisplayChange = 0;
        static bool showTemp = true;
        
        if (millis() - lastDisplayChange > 2000) {
            if (showTemp) {
                display.display(static_cast<int>(temperature));
            } else {
                display.display(static_cast<int>(humidity));
            }
            showTemp = !showTemp;
            lastDisplayChange = millis();
        }
    }
}

void DeviceManager::setLedColor(uint8_t r, uint8_t g, uint8_t b) {
    if (deviceHealth.ledStripHealthy) {
        fill_solid(leds, 16, CRGB(r, g, b));
        FastLED.show();
    }
}

bool DeviceManager::isSystemHealthy() const {
    return sensorData.systemHealthy;
}

void DeviceManager::checkDeviceHealth() {
    // Проверка BME280
    deviceHealth.bme280Healthy = bme.begin(0x76, &Wire);
    
    // Проверка BH1750
    deviceHealth.bh1750Healthy = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
    
    // Проверка почвенных датчиков
    float moisture = readSoilMoisture();
    deviceHealth.soilSensorHealthy = (moisture >= 0 && moisture <= 100);
    
    // Обновление общего состояния
    sensorData.systemHealthy = deviceHealth.bme280Healthy && 
                              deviceHealth.bh1750Healthy && 
                              deviceHealth.soilSensorHealthy;
}