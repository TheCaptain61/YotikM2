#ifndef AUTOMATION_H
#define AUTOMATION_H

#include "Config.h"
#include "DeviceManager.h"

class Automation {
public:
    void process(const SensorData& data, const SystemSettings& settings, DeviceManager& devices);
    
private:
    void controlTemperature(const SensorData& data, const SystemSettings& settings, DeviceManager& devices);
    void controlHumidity(const SensorData& data, const SystemSettings& settings, DeviceManager& devices);
    void controlSoilMoisture(const SensorData& data, const SystemSettings& settings, DeviceManager& devices);
    void controlLighting(const SensorData& data, const SystemSettings& settings, DeviceManager& devices);
    void controlVentilation(const SensorData& data, const SystemSettings& settings, DeviceManager& devices);
    
    unsigned long lastPumpRun = 0;
    const unsigned long PUMP_COOLDOWN = 300000; // 5 minutes
};

#endif