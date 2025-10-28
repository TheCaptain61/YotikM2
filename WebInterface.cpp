#include "WebInterface.h"
#include "DeviceManager.h"
#include "ConfigManager.h"

extern DeviceManager deviceManager;
extern ConfigManager configManager;

WebInterface::WebInterface() : server(80) {}

void WebInterface::begin() {
    setupRoutes();
    server.begin();
}

void WebInterface::handleClient() {
    server.handleClient();
}

void WebInterface::setupRoutes() {
    server.on("/", HTTP_GET, [this]() {
        handleRoot();
    });
    
    server.on("/api/sensor-data", HTTP_GET, [this]() {
        handleSensorData();
    });
    
    server.on("/api/system-info", HTTP_GET, [this]() {
        handleSystemInfo();
    });
    
    server.on("/api/settings", HTTP_GET, [this]() {
        handleGetSettings();
    });
    
    server.on("/api/settings", HTTP_POST, [this]() {
        handlePostSettings();
    });
    
    server.on("/api/control", HTTP_POST, [this]() {
        handleControl();
    });
    
    server.onNotFound([this]() {
        handleNotFound();
    });
}

void WebInterface::handleRoot() {
    String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
        <title>Smart Greenhouse</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body { font-family: Arial, sans-serif; margin: 20px; }
            .card { border: 1px solid #ddd; padding: 15px; margin: 10px 0; border-radius: 5px; }
            .healthy { color: green; }
            .error { color: red; }
            .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }
        </style>
    </head>
    <body>
        <h1>Smart Greenhouse Control</h1>
        <div id="sensorData"></div>
        <script>
            async function updateSensorData() {
                const response = await fetch('/api/sensor-data');
                const data = await response.json();
                document.getElementById('sensorData').innerHTML = `
                    <div class="grid">
                        <div class="card">
                            <h3>Temperature</h3>
                            <p>Air: ${data.airTemperature}°C</p>
                            <p>Soil: ${data.soilTemperature}°C</p>
                        </div>
                        <div class="card">
                            <h3>Humidity</h3>
                            <p>Air: ${data.airHumidity}%</p>
                            <p>Soil: ${data.soilMoisture}%</p>
                        </div>
                        <div class="card">
                            <h3>Environment</h3>
                            <p>Light: ${data.lightLevel} lux</p>
                            <p>Pressure: ${data.pressure} hPa</p>
                        </div>
                        <div class="card">
                            <h3>Devices</h3>
                            <p>Heater: ${data.heaterState ? 'ON' : 'OFF'}</p>
                            <p>Light: ${data.lightState ? 'ON' : 'OFF'}</p>
                            <p>Ventilation: ${data.ventilationState ? 'ON' : 'OFF'}</p>
                            <p>Water Pump: ${data.waterPumpState ? 'ON' : 'OFF'}</p>
                        </div>
                        <div class="card">
                            <h3>System Status</h3>
                            <p class="${data.systemHealthy ? 'healthy' : 'error'}">
                                System: ${data.systemHealthy ? 'HEALTHY' : 'ERROR'}
                            </p>
                        </div>
                    </div>
                `;
            }
            setInterval(updateSensorData, 5000);
            updateSensorData();
        </script>
    </body>
    </html>
    )";
    server.send(200, "text/html", html);
}

void WebInterface::handleSensorData() {
    server.send(200, "application/json", getSensorDataJSON());
}

void WebInterface::handleSystemInfo() {
    server.send(200, "application/json", getSystemInfoJSON());
}

void WebInterface::handleGetSettings() {
    DeviceSettings settings = configManager.getSettings();
    
    DynamicJsonDocument doc(1024);
    doc["tempSetpoint"] = settings.tempSetpoint;
    doc["tempHysteresis"] = settings.tempHysteresis;
    doc["lightThreshold"] = settings.lightThreshold;
    doc["ventilationTemp"] = settings.ventilationTemp;
    doc["ventilationHumidity"] = settings.ventilationHumidity;
    doc["soilMoistureThreshold"] = settings.soilMoistureThreshold;
    doc["wateringDuration"] = settings.wateringDuration;
    doc["sensorReadInterval"] = settings.sensorReadInterval;
    doc["controlUpdateInterval"] = settings.controlUpdateInterval;
    doc["deviceName"] = settings.deviceName;
    
    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
}

void WebInterface::handlePostSettings() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, body);
        
        if (!error) {
            DeviceSettings newSettings = configManager.getSettings();
            
            if (doc.containsKey("tempSetpoint")) newSettings.tempSetpoint = doc["tempSetpoint"];
            if (doc.containsKey("tempHysteresis")) newSettings.tempHysteresis = doc["tempHysteresis"];
            if (doc.containsKey("lightThreshold")) newSettings.lightThreshold = doc["lightThreshold"];
            if (doc.containsKey("ventilationTemp")) newSettings.ventilationTemp = doc["ventilationTemp"];
            if (doc.containsKey("ventilationHumidity")) newSettings.ventilationHumidity = doc["ventilationHumidity"];
            if (doc.containsKey("soilMoistureThreshold")) newSettings.soilMoistureThreshold = doc["soilMoistureThreshold"];
            if (doc.containsKey("wateringDuration")) newSettings.wateringDuration = doc["wateringDuration"];
            if (doc.containsKey("sensorReadInterval")) newSettings.sensorReadInterval = doc["sensorReadInterval"];
            if (doc.containsKey("controlUpdateInterval")) newSettings.controlUpdateInterval = doc["controlUpdateInterval"];
            if (doc.containsKey("deviceName")) newSettings.deviceName = doc["deviceName"].as<String>();
            
            configManager.saveSettings(newSettings);
            server.send(200, "application/json", "{\"status\":\"success\"}");
        } else {
            server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
        }
    } else {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data\"}");
    }
}

void WebInterface::handleControl() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, body);
        
        if (!error) {
            if (doc.containsKey("heater")) {
                deviceManager.setHeater(doc["heater"]);
            }
            if (doc.containsKey("light")) {
                deviceManager.setLight(doc["light"]);
            }
            if (doc.containsKey("ventilation")) {
                deviceManager.setVentilation(doc["ventilation"]);
            }
            if (doc.containsKey("waterPump")) {
                deviceManager.setWaterPump(doc["waterPump"]);
            }
            
            server.send(200, "application/json", "{\"status\":\"success\"}");
        } else {
            server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
        }
    } else {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data\"}");
    }
}

void WebInterface::handleNotFound() {
    server.send(404, "text/plain", "Not Found");
}

String WebInterface::getSensorDataJSON() {
    DynamicJsonDocument doc(1024);
    SensorData data = deviceManager.getSensorData();

    doc["airTemperature"] = data.airTemperature;
    doc["airHumidity"] = data.airHumidity;
    doc["pressure"] = data.pressure;
    doc["lightLevel"] = data.lightLevel;
    doc["soilMoisture"] = data.soilMoisture;
    doc["soilTemperature"] = data.soilTemperature;
    doc["heaterState"] = data.heaterState;
    doc["lightState"] = data.lightState;
    doc["ventilationState"] = data.ventilationState;
    doc["waterPumpState"] = data.waterPumpState;
    doc["systemHealthy"] = data.systemHealthy;
    doc["errorMessage"] = data.errorMessage;
    doc["lastUpdate"] = data.lastUpdate;

    String output;
    serializeJson(doc, output);
    return output;
}

String WebInterface::getSystemInfoJSON() {
    DynamicJsonDocument doc(512);
    DeviceHealth health = deviceManager.getDeviceHealth();
    
    doc["bme280Healthy"] = health.bme280Healthy;
    doc["bh1750Healthy"] = health.bh1750Healthy;
    doc["soilSensorHealthy"] = health.soilSensorHealthy;
    doc["displayHealthy"] = health.displayHealthy;
    doc["ledStripHealthy"] = health.ledStripHealthy;
    doc["lastUpdateSuccess"] = health.lastUpdateSuccess;
    doc["systemHealthy"] = deviceManager.isSystemHealthy();
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["uptime"] = millis() / 1000;

    String output;
    serializeJson(doc, output);
    return output;
}