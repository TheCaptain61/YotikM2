#include "WebInterface.h"
#include "Config.h"
#include "DeviceManager.h"
#include "GlobalInstances.h"
#include "WiFi.h"


void WebInterface::begin(WebServer& srv) {
    server = &srv;
    
    Serial.println("🌐 Starting Web Interface...");
    
    // Setup routes with diagnostics
    server->on("/", HTTP_GET, [this]() { 
        Serial.println("📨 GET / request received");
        handleRoot(); 
    });
    
    server->on("/api/sensors", HTTP_GET, [this]() { 
        Serial.println("📨 GET /api/sensors request received");
        handleSensorData(); 
    });
    
    server->on("/api/settings", HTTP_GET, [this]() { 
        Serial.println("📨 GET /api/settings request received");
        handleSettings(); 
    });
    
    server->on("/api/settings", HTTP_POST, [this]() { 
        Serial.println("📨 POST /api/settings request received");
        handleSettings(); 
    });
    
    server->on("/api/control", HTTP_POST, [this]() { 
        Serial.println("📨 POST /api/control request received");
        handleControl(); 
    });
    
    server->on("/api/system", HTTP_GET, [this]() { 
        Serial.println("📨 GET /api/system request received");
        handleSystemInfo(); 
    });
    
    server->on("/api/calibrate", HTTP_POST, [this]() { 
        Serial.println("📨 POST /api/calibrate request received");
        handleCalibrate(); 
    });
    
    server->on("/api/reset", HTTP_POST, [this]() { 
        Serial.println("📨 POST /api/reset request received");
        handleReset(); 
    });
    
    // Test endpoint
    server->on("/test", HTTP_GET, [this]() {
        Serial.println("📨 GET /test request received");
        server->send(200, "text/plain", "Web server is working! IP: " + WiFi.localIP().toString());
    });
    
    // Serve static files (CSS)
    server->on("/style.css", HTTP_GET, [this]() {
        Serial.println("📨 GET /style.css request received");
        String css = R"(
            body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: #f5f5f5; }
            .container { max-width: 1200px; margin: 0 auto; }
            .card { background: white; padding: 20px; margin: 10px 0; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
            .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }
            .sensor-value { font-size: 24px; font-weight: bold; color: #2c3e50; }
            .control { margin: 10px 0; }
            button { padding: 10px 15px; margin: 5px; border: none; border-radius: 5px; cursor: pointer; font-size: 14px; }
            .btn-on { background: #27ae60; color: white; }
            .btn-off { background: #e74c3c; color: white; }
            .btn-auto { background: #3498db; color: white; }
            .btn-neutral { background: #95a5a6; color: white; }
            .status-on { color: #27ae60; font-weight: bold; }
            .status-off { color: #e74c3c; font-weight: bold; }
            .form-group { margin: 10px 0; }
            label { display: block; margin: 5px 0; font-weight: bold; }
            input[type="number"], input[type="text"], input[type="password"] { 
                width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; 
            }
            input[type="checkbox"] { margin-right: 10px; }
            .device-status { padding: 5px 10px; border-radius: 3px; display: inline-block; }
            .healthy { background: #d4edda; color: #155724; }
            .unhealthy { background: #f8d7da; color: #721c24; }
        )";
        server->send(200, "text/css", css);
    });
    
    server->onNotFound([this]() {
        Serial.println("❌ 404 - Not Found: " + server->uri());
        server->send(404, "text/plain", "Endpoint not found: " + server->uri());
    });
    
    server->begin();
    Serial.println("✅ Web Interface initialized");
    Serial.println("📍 Available endpoints:");
    Serial.println("   http://" + WiFi.localIP().toString() + "/");
    Serial.println("   http://" + WiFi.localIP().toString() + "/test");
    Serial.println("   http://" + WiFi.localIP().toString() + "/api/sensors");
    Serial.println("   http://" + WiFi.localIP().toString() + "/style.css");
}

void WebInterface::handleRoot() {
    Serial.println("🔄 Generating main page...");
    sendHTMLResponse(200, getSystemHTML());
}

void WebInterface::handleSensorData() {
    Serial.println("📊 Sending sensor data...");
    sendJSONResponse(200, "OK", getSensorDataJSON());
}

void WebInterface::handleSettings() {
    if (server->method() == HTTP_GET) {
        Serial.println("⚙️ Sending settings...");
        sendJSONResponse(200, "OK", getSettingsJSON());
    } else if (server->method() == HTTP_POST) {
        Serial.println("💾 Updating settings...");
        String body = server->arg("plain");
        Serial.println("Received: " + body);
        
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            Serial.println("❌ JSON parse error: " + String(error.c_str()));
            sendJSONResponse(400, "Invalid JSON: " + String(error.c_str()));
            return;
        }
        
        // Update settings
        bool updated = false;
        if (doc.containsKey("tempSetpoint")) {
            systemSettings.tempSetpoint = doc["tempSetpoint"];
            Serial.println("Updated tempSetpoint: " + String(systemSettings.tempSetpoint));
            updated = true;
        }
        if (doc.containsKey("humSetpoint")) {
            systemSettings.humSetpoint = doc["humSetpoint"];
            Serial.println("Updated humSetpoint: " + String(systemSettings.humSetpoint));
            updated = true;
        }
        if (doc.containsKey("soilMoistureSetpoint")) {
            systemSettings.soilMoistureSetpoint = doc["soilMoistureSetpoint"];
            Serial.println("Updated soilMoistureSetpoint: " + String(systemSettings.soilMoistureSetpoint));
            updated = true;
        }
        if (doc.containsKey("lightOnHour")) {
            systemSettings.lightOnHour = doc["lightOnHour"];
            Serial.println("Updated lightOnHour: " + String(systemSettings.lightOnHour));
            updated = true;
        }
        if (doc.containsKey("lightOffHour")) {
            systemSettings.lightOffHour = doc["lightOffHour"];
            Serial.println("Updated lightOffHour: " + String(systemSettings.lightOffHour));
            updated = true;
        }
        if (doc.containsKey("automationEnabled")) {
            systemSettings.automationEnabled = doc["automationEnabled"];
            Serial.println("Updated automationEnabled: " + String(systemSettings.automationEnabled));
            updated = true;
        }
        if (doc.containsKey("wifiSSID")) {
            strlcpy(systemSettings.wifiSSID, doc["wifiSSID"], sizeof(systemSettings.wifiSSID));
            Serial.println("Updated wifiSSID: " + String(systemSettings.wifiSSID));
            updated = true;
        }
        if (doc.containsKey("wifiPassword")) {
            strlcpy(systemSettings.wifiPassword, doc["wifiPassword"], sizeof(systemSettings.wifiPassword));
            Serial.println("Updated wifiPassword: [hidden]");
            updated = true;
        }
        
        if (updated) {
            sendJSONResponse(200, "Settings updated successfully");
        } else {
            sendJSONResponse(400, "No valid settings received");
        }
    }
}

void WebInterface::handleControl() {
    String body = server->arg("plain");
    Serial.println("🎛️ Control command: " + body);
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
        Serial.println("❌ JSON parse error: " + String(error.c_str()));
        sendJSONResponse(400, "Invalid JSON");
        return;
    }
    
    if (!doc.containsKey("device") || !doc.containsKey("state")) {
        Serial.println("❌ Missing device or state in control command");
        sendJSONResponse(400, "Missing device or state");
        return;
    }
    
    String device = doc["device"];
    bool state = doc["state"];
    
    Serial.println("Control: " + device + " -> " + String(state));
    
    if (!validateControlCommand(device, state)) {
        Serial.println("❌ Invalid control command");
        sendJSONResponse(400, "Invalid device or state");
        return;
    }
    
    applyControlCommand(device, state);
    sendJSONResponse(200, "Control command executed: " + device + " " + (state ? "ON" : "OFF"));
}

void WebInterface::handleSystemInfo() {
    Serial.println("🔍 Sending system info...");
    sendJSONResponse(200, "OK", getSystemInfoJSON());
}

void WebInterface::handleCalibrate() {
    String type = server->arg("type");
    Serial.println("🎯 Calibration request: " + type);
    
    if (type == "air") {
        deviceManager.calibrateSoilSensor(false);
        sendJSONResponse(200, "Air calibration completed");
    } else if (type == "water") {
        deviceManager.calibrateSoilSensor(true);
        sendJSONResponse(200, "Water calibration completed");
    } else {
        sendJSONResponse(400, "Invalid calibration type. Use 'air' or 'water'");
    }
}

void WebInterface::handleReset() {
    String type = server->arg("type");
    Serial.println("🔄 Reset request: " + type);
    
    if (type == "settings") {
        SystemSettings defaults;
        systemSettings = defaults;
        sendJSONResponse(200, "Settings reset to defaults");
    } else if (type == "wifi") {
        strlcpy(systemSettings.wifiSSID, "", sizeof(systemSettings.wifiSSID));
        strlcpy(systemSettings.wifiPassword, "", sizeof(systemSettings.wifiPassword));
        sendJSONResponse(200, "WiFi settings reset");
    } else {
        sendJSONResponse(400, "Invalid reset type. Use 'settings' or 'wifi'");
    }
}

void WebInterface::sendJSONResponse(int code, const String& message, const String& jsonData) {
    String output;
    if (jsonData.length() > 0) {
        output = jsonData;
    } else {
        DynamicJsonDocument doc(256);
        doc["status"] = code;
        doc["message"] = message;
        serializeJson(doc, output);
    }
    
    Serial.println("📤 Sending JSON response: " + String(code) + " - " + message);
    server->send(code, "application/json", output);
}

void WebInterface::sendHTMLResponse(int code, const String& html) {
    Serial.println("📤 Sending HTML response, length: " + String(html.length()));
    server->send(code, "text/html", html);
}

String WebInterface::getSystemHTML() {
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Greenhouse</title>
    <style>
        body{font-family:Arial;margin:20px;background:#f0f0f0;}
        .card{background:white;padding:15px;margin:10px 0;border-radius:5px;}
        button{padding:8px 12px;margin:2px;border:none;border-radius:3px;}
        .btn-on{background:#4CAF50;color:white;}
        .btn-off{background:#f44336;color:white;}
        .btn-neutral{background:#2196F3;color:white;}
        .sensor-value{font-weight:bold;color:#333;}
    </style>
</head>
<body>
    <h1>Smart Greenhouse</h1>
    
    <div class="card">
        <h3>Sensors</h3>
        <div id="sensorData">Loading...</div>
        <button onclick="refreshData()">Refresh</button>
    </div>
    
    <div class="card">
        <h3>Control</h3>
        <button class="btn-on" onclick="controlDevice('pump', true)">Pump ON</button>
        <button class="btn-off" onclick="controlDevice('pump', false)">Pump OFF</button>
        <br>
        <button class="btn-on" onclick="controlDevice('fan', true)">Fan ON</button>
        <button class="btn-off" onclick="controlDevice('fan', false)">Fan OFF</button>
        <br>
        <button class="btn-on" onclick="controlDevice('light', true)">Light ON</button>
        <button class="btn-off" onclick="controlDevice('light', false)">Light OFF</button>
        <br>
        <!-- ИСПРАВЛЕННЫЕ КНОПКИ ДВЕРИ -->
        <button class="btn-neutral" onclick="controlDevice('door', 0)">Open Door</button>
        <button class="btn-neutral" onclick="controlDevice('door', 90)">Close Door</button>
        <button class="btn-neutral" onclick="controlDevice('door', 180)">Ventilation</button>
    </div>

    <script>
        function refreshData(){
            fetch('/api/sensors').then(r=>r.json()).then(data=>{
                let html='';
                if(data.airTemperature) html+='<p>Air: '+data.airTemperature.toFixed(1)+'C</p>';
                if(data.airHumidity) html+='<p>Humidity: '+data.airHumidity.toFixed(1)+'%</p>';
                if(data.soilMoisture) html+='<p>Soil: '+data.soilMoisture.toFixed(1)+'%</p>';
                document.getElementById('sensorData').innerHTML=html;
            });
        }
        
        function controlDevice(device, state){
            fetch('/api/control',{
                method:'POST',
                headers:{'Content-Type':'application/json'},
                body:JSON.stringify({device:device, state:state})
            }).then(r=>r.json()).then(data=>alert(data.message));
        }
        
        setInterval(refreshData,5000);
        refreshData();
    </script>
</body>
</html>
)=====";
    return html;
}

String WebInterface::getSensorDataJSON() {
    DynamicJsonDocument doc(1024);
    
    if (!isnan(sensorData.airTemperature))
        doc["airTemperature"] = sensorData.airTemperature;
    if (!isnan(sensorData.airHumidity))
        doc["airHumidity"] = sensorData.airHumidity;
    if (!isnan(sensorData.pressure))
        doc["pressure"] = sensorData.pressure;
    if (!isnan(sensorData.soilTemperature))
        doc["soilTemperature"] = sensorData.soilTemperature;
    if (!isnan(sensorData.soilMoisture))
        doc["soilMoisture"] = sensorData.soilMoisture;
    if (!isnan(sensorData.lightLevel))
        doc["lightLevel"] = sensorData.lightLevel;
    
    doc["pumpState"] = sensorData.pumpState;
    doc["fanState"] = sensorData.fanState;
    doc["heaterState"] = sensorData.heaterState;
    doc["lightState"] = sensorData.lightState;
    doc["doorState"] = sensorData.doorState;
    
    String output;
    serializeJson(doc, output);
    return output;
}

String WebInterface::getSettingsJSON() {
    DynamicJsonDocument doc(512);
    
    doc["tempSetpoint"] = systemSettings.tempSetpoint;
    doc["humSetpoint"] = systemSettings.humSetpoint;
    doc["soilMoistureSetpoint"] = systemSettings.soilMoistureSetpoint;
    doc["lightOnHour"] = systemSettings.lightOnHour;
    doc["lightOffHour"] = systemSettings.lightOffHour;
    doc["automationEnabled"] = systemSettings.automationEnabled;
    doc["wifiSSID"] = systemSettings.wifiSSID;
    
    String output;
    serializeJson(doc, output);
    return output;
}

String WebInterface::getSystemInfoJSON() {
    DynamicJsonDocument doc(512);
    
    doc["systemHealthy"] = sensorData.systemHealthy;
    doc["bme280Healthy"] = deviceConfig.bme280Healthy;
    doc["bh1750Healthy"] = deviceConfig.bh1750Healthy;
    doc["soilSensorsHealthy"] = deviceConfig.soilSensorsHealthy;
    doc["deviceSummary"] = deviceManager.getDeviceSummary();
    
    String output;
    serializeJson(doc, output);
    return output;
}

bool WebInterface::validateControlCommand(const String& device, bool state) {
    if (device == "pump" || device == "fan" || device == "light") {
        return true;
    }
    if (device == "door") {
        // Для двери разрешаем углы 0, 90, 180
        return (state == 0 || state == 90 || state == 180);
    }
    return false;
}

void WebInterface::applyControlCommand(const String& device, bool state) {
    if (device == "pump") {
        deviceManager.controlPump(state);
    } else if (device == "fan") {
        deviceManager.controlFan(state);
    } else if (device == "light") {
        deviceManager.controlLight(state);
    } else if (device == "door") {
        // Для двери state - это угол (0, 90, 180)
        deviceManager.controlDoor(state);
    }
}