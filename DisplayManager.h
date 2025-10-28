#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "Config.h"

class DisplayManager {
public:
    void begin();
    void updateDisplay(const SensorData& data, const SystemSettings& settings);
    void showMessage(const String& message);
    void showNumber(int number, bool leadingZero = true);
    void showTemperature(float temp);
    void showHumidity(float hum);
    void showLoading(uint8_t step);
    void setBrightness(uint8_t brightness);
    void clear();
    
private:
    void showNextMode(const SensorData& data, const SystemSettings& settings);
    void showError(const String& error);
    
    unsigned long lastModeChange = 0;
    uint8_t currentMode = 0;
    uint8_t displayModes = 4;
    
    // Используем другие имена для сегментов чтобы избежать конфликтов
    static const uint8_t SEG_DEGREE[];
    static const uint8_t SEG_CELSIUS[];
    static const uint8_t SEG_PERCENT[];
};
#endif