#ifndef GLOBAL_INSTANCES_H
#define GLOBAL_INSTANCES_H

// Включаем ВСЕ заголовочные файлы для полных определений
#include "Config.h"
#include "DeviceManager.h"
#include "DisplayManager.h"
#include "EEPROMManager.h"
#include "WebInterface.h"
#include "Automation.h"

// Объявления extern
extern DeviceManager deviceManager;
extern DisplayManager displayManager;
extern EEPROMManager eepromManager;
extern WebInterface webInterface;
extern Automation automation;

#endif