#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include <EEPROM.h>
#include "Config.h"

class EEPROMManager {
public:
    void begin();
    bool loadSettings(SystemSettings& settings);
    bool saveSettings(const SystemSettings& settings);
    void resetToDefaults();
    void printSettings(const SystemSettings& settings);
    
private:
    bool validateSettings(const SystemSettings& settings);
    void migrateSettings(SystemSettings& settings, uint8_t fromVersion);
    
    static constexpr int SETTINGS_ADDRESS = 0;
};

#endif