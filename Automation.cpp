#include "Automation.h"
#include "DeviceManager.h"
#include "Config.h"



void Automation::updateTemperatureControl() {
    SensorData data = deviceManager.getSensorData();
    DeviceSettings settings = configManager.getSettings();
    
    float temp = data.airTemperature;
    float setpoint = settings.tempSetpoint;
    float hysteresis = settings.tempHysteresis;

    if (temp > setpoint + hysteresis) {
        // Выключить обогреватель
        deviceManager.setHeater(false);
    } else if (temp < setpoint - hysteresis) {
        // Включить обогреватель
        deviceManager.setHeater(true);
    }
    // В зоне гистерезиса - оставить текущее состояние
}

void Automation::updateLightControl() {
    SensorData data = deviceManager.getSensorData();
    DeviceSettings settings = configManager.getSettings();

    if (data.lightLevel < settings.lightThreshold) {
        deviceManager.setLight(true);
    } else {
        deviceManager.setLight(false);
    }
}

void Automation::updateVentilationControl() {
    SensorData data = deviceManager.getSensorData();
    DeviceSettings settings = configManager.getSettings();

    if (data.airTemperature > settings.ventilationTemp || 
        data.airHumidity > settings.ventilationHumidity) {
        deviceManager.setVentilation(true);
    } else {
        deviceManager.setVentilation(false);
    }
}

void Automation::updateWateringControl() {
    SensorData data = deviceManager.getSensorData();
    DeviceSettings settings = configManager.getSettings();

    if (wateringInProgress) {
        if (millis() - lastWateringTime > settings.wateringDuration) {
            deviceManager.setWaterPump(false);
            wateringInProgress = false;
        }
    } else {
        if (data.soilMoisture < settings.soilMoistureThreshold) {
            deviceManager.setWaterPump(true);
            wateringInProgress = true;
            lastWateringTime = millis();
        }
    }
}

void Automation::updateAllSystems() {
    updateTemperatureControl();
    updateLightControl();
    updateVentilationControl();
    updateWateringControl();
}