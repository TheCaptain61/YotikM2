#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>
#include <ArduinoJson.h>
#include "Config.h"

class WebInterface {
public:
    WebInterface();
    void begin();
    void handleClient();
    
private:
    WebServer server;
    
    void setupRoutes();
    void handleRoot();
    void handleSensorData();
    void handleSystemInfo();
    void handleGetSettings();
    void handlePostSettings();
    void handleControl();
    void handleNotFound();
    
    String getSensorDataJSON();
    String getSystemInfoJSON();
};

#endif