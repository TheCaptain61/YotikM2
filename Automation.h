#ifndef AUTOMATION_H
#define AUTOMATION_H

#include "Config.h"

class Automation {
public:
    void updateTemperatureControl();
    void updateLightControl();
    void updateVentilationControl();
    void updateWateringControl();
    void updateAllSystems();
    
private:
    unsigned long lastWateringTime = 0;
    bool wateringInProgress = false;
};

#endif