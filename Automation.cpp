#include "Automation.h"
#include "Config.h"
#include "GlobalInstances.h"

void Automation::process(const SensorData& data, const SystemSettings& settings, DeviceManager& devices) {
    if (!settings.automationEnabled) return;
    
    controlTemperature(data, settings, devices);
    controlHumidity(data, settings, devices);
    controlSoilMoisture(data, settings, devices);
    controlLighting(data, settings, devices);
    controlVentilation(data, settings, devices);
}

void Automation::controlTemperature(const SensorData& data, const SystemSettings& settings, DeviceManager& devices) {
    if (isnan(data.airTemperature)) return;
    
    float temp = data.airTemperature;
    float setpoint = settings.tempSetpoint;
    float hysteresis = 1.0;
    
    if (temp > setpoint + hysteresis) {
        // Слишком жарко - включаем вентилятор
        devices.controlFan(true);
    } else if (temp < setpoint - hysteresis) {
        // Слишком холодно - выключаем вентилятор
        devices.controlFan(false);
        // Обогревателя нет, поэтому просто выключаем вентилятор
    } else {
        // В норме - выключаем вентилятор
        devices.controlFan(false);
    }
}
    
    float temp = data.airTemperature;
    float setpoint = settings.tempSetpoint;
    float hysteresis = 1.0;
    
    if (temp > setpoint + hysteresis) {
        // Too hot - turn on fan, turn off heater
        devices.controlFan(true);
        devices.controlHeater(false);
    } else if (temp < setpoint - hysteresis) {
        // Too cold - turn on heater, turn off fan
        devices.controlHeater(true);
        devices.controlFan(false);
    } else {
        // Within range - turn both off
        devices.controlFan(false);
        devices.controlHeater(false);
    }
}

void Automation::controlHumidity(const SensorData& data, const SystemSettings& settings, DeviceManager& devices) {
    if (isnan(data.airHumidity)) return;
    
    float humidity = data.airHumidity;
    float setpoint = settings.humSetpoint;
    float hysteresis = 5.0;
    
    if (humidity > setpoint + hysteresis) {
        // Too humid - increase ventilation
        devices.controlFan(true);
    } else if (humidity < setpoint - hysteresis) {
        // Too dry -可以考虑 добавить увлажнитель в будущем
        devices.controlFan(false);
    }
}

void Automation::controlSoilMoisture(const SensorData& data, const SystemSettings& settings, DeviceManager& devices) {
    if (isnan(data.soilMoisture)) return;
    
    float moisture = data.soilMoisture;
    float setpoint = settings.soilMoistureSetpoint;
    unsigned long currentTime = millis();
    
    // Проверяем, нужно ли поливать и прошло ли достаточно времени с последнего полива
    if (moisture < setpoint - 5.0 && (currentTime - lastPumpRun) > PUMP_COOLDOWN) {
        // Полив в течение 5 секунд
        devices.controlPump(true, 5000);
        lastPumpRun = currentTime;
        Serial.println("💧 Automated watering started");
    }
}

void Automation::controlLighting(const SensorData& data, const SystemSettings& settings, DeviceManager& devices) {
    // Простое управление по времени (можно улучшить с учетом освещенности)
    int currentHour = 12; // В реальности получать из NTPClient
    
    if (currentHour >= settings.lightOnHour && currentHour < settings.lightOffHour) {
        devices.controlLight(true);
    } else {
        devices.controlLight(false);
    }
}

void Automation::controlVentilation(const SensorData& data, const SystemSettings& settings, DeviceManager& devices) {
    // Дополнительная вентиляция при высокой температуре и влажности
    if (!isnan(data.airTemperature) && !isnan(data.airHumidity)) {
        if (data.airTemperature > 28.0 && data.airHumidity > 70.0) {
            devices.controlFan(true);
        }
    }
}