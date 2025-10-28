#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include "Config.h"

class DeviceManager {
public:
    DeviceManager();
    void begin();
    void readAllSensors();
    void checkDeviceHealth();
    void rediscoverDevices();
    
    void controlPump(bool state, unsigned long duration = 0);
    void controlFan(bool state);
    void controlHeater(bool state);
    void controlLight(bool state);
    void controlDoor(bool angle);
    void stopAllDevices();
    
    void calibrateSoilSensor(bool inWater);
    String getDeviceSummary() const;
    bool isSystemHealthy() const;
    
private:
    void initializePins();
    void discoverI2CDevices();
    void initializeDetectedDevices();
    bool initializeBME280(uint8_t address);
    bool initializeBH1750(uint8_t address);
    bool initializeSoilSensors();
    void initializeLEDMatrix();
    void identifyUnknownDevice(uint8_t address);
    
    void readBME280();
    void readBH1750();
    void readSoilSensors();
    
    bool devicesInitialized = false;
    unsigned long pumpStartTime = 0;
    unsigned long pumpDuration = 0;
    bool pumpAutoStop = false;
    
    uint16_t bme280ErrorCount = 0;
    uint16_t bh1750ErrorCount = 0;
    uint16_t soilSensorErrorCount = 0;
};

#endif