#include "EEPROMManager.h"
#include "Config.h"
#include "GlobalInstances.h"

void EEPROMManager::begin() {
    EEPROM.begin(EEPROM_SIZE);
    Serial.println("✅ EEPROM Manager initialized");
}

bool EEPROMManager::loadSettings(SystemSettings& settings) {
    Serial.println("📖 Loading settings from EEPROM...");
    
    EEPROM.get(SETTINGS_ADDRESS, settings);
    
    if (!validateSettings(settings)) {
        Serial.println("❌ Invalid settings in EEPROM, using defaults");
        return false;
    }
    
    // Миграция настроек при необходимости
    if (settings.version != CONFIG_VERSION) {
        Serial.printf("🔄 Migrating settings from version %d to %d\n", 
                     settings.version, CONFIG_VERSION);
        migrateSettings(settings, settings.version);
        saveSettings(settings);
    }
    
    Serial.println("✅ Settings loaded successfully");
    printSettings(settings);
    return true;
}

bool EEPROMManager::saveSettings(const SystemSettings& settings) {
    if (!validateSettings(settings)) {
        Serial.println("❌ Invalid settings, not saving");
        return false;
    }
    
    EEPROM.put(SETTINGS_ADDRESS, settings);
    bool success = EEPROM.commit();
    
    if (success) {
        Serial.println("💾 Settings saved successfully");
    } else {
        Serial.println("❌ Failed to save settings to EEPROM");
    }
    
    return success;
}

bool EEPROMManager::validateSettings(const SystemSettings& settings) {
    if (settings.version != CONFIG_VERSION) {
        return false;
    }
    
    if (settings.tempSetpoint < 10 || settings.tempSetpoint > 40) {
        return false;
    }
    
    if (settings.humSetpoint < 20 || settings.humSetpoint > 90) {
        return false;
    }
    
    if (settings.soilMoistureSetpoint < 10 || settings.soilMoistureSetpoint > 90) {
        return false;
    }
    
    if (settings.lightOnHour < 0 || settings.lightOnHour > 23 ||
        settings.lightOffHour < 0 || settings.lightOffHour > 23) {
        return false;
    }
    
    return true;
}

void EEPROMManager::migrateSettings(SystemSettings& settings, uint8_t fromVersion) {
    switch(fromVersion) {
        case 1:
            // Миграция с версии 1 на 2
            settings.displayBrightness = 7;
            settings.use24HourFormat = true;
            settings.version = 2;
            // break; // Продолжаем миграцию
            
        case 2:
            // Миграция с версии 2 на 3
            settings.soilMoistureSetpoint = 50.0;
            settings.version = 3;
            break;
            
        default:
            // Неизвестная версия - сброс к defaults
            resetToDefaults();
            break;
    }
}

void EEPROMManager::resetToDefaults() {
    SystemSettings defaults;
    saveSettings(defaults);
    Serial.println("🔄 EEPROM reset to default values");
}

void EEPROMManager::printSettings(const SystemSettings& settings) {
    Serial.println("\n=== Current Settings ===");
    Serial.printf("Version: %d\n", settings.version);
    Serial.printf("WiFi SSID: %s\n", settings.wifiSSID);
    Serial.printf("Temperature Setpoint: %.1f°C\n", settings.tempSetpoint);
    Serial.printf("Humidity Setpoint: %.1f%%\n", settings.humSetpoint);
    Serial.printf("Soil Moisture Setpoint: %.1f%%\n", settings.soilMoistureSetpoint);
    Serial.printf("Light Schedule: %02d:00 - %02d:00\n", 
                  settings.lightOnHour, settings.lightOffHour);
    Serial.printf("Automation: %s\n", settings.automationEnabled ? "Enabled" : "Disabled");
    Serial.printf("Display Brightness: %d\n", settings.displayBrightness);
    Serial.println("========================\n");
}