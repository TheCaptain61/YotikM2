#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>
#include <ArduinoJson.h>
#include "Config.h"

// Forward declarations
class DeviceManager;
extern DeviceManager deviceManager;

class WebInterface {
public:
    void begin(WebServer& server);
    
    // API endpoints
    void handleRoot();
    void handleSensorData();
    void handleSettings();
    void handleControl();
    void handleSystemInfo();
    void handleCalibrate();
    void handleReset();
    
private:
    WebServer* server;
    
    void sendJSONResponse(int code, const String& message, const String& jsonData = "");
    void sendHTMLResponse(int code, const String& html);
    String getSystemHTML();
    String getSensorDataJSON();
    String getSettingsJSON();
    String getSystemInfoJSON();
    bool validateControlCommand(const String& device, bool state);
    void applyControlCommand(const String& device, bool state);
};


#endif