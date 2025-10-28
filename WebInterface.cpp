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
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Greenhouse M2</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <h1>Smart Greenhouse M2</h1>
        <p><strong>IP:</strong> )=====";
    html += WiFi.localIP().toString();
    html += R"=====(</p>
        
        <div class="grid">
            <!-- Sensor Data -->
            <div class="card">
                <h2>Sensor Data</h2>
                <div id="sensorData">
                    <p>Loading...</p>
                </div>
                <button onclick="refreshData()">Refresh</button>
                <button onclick="location.reload()">Reload Page</button>
            </div>
            
            <!-- Device Control -->
            <div class="card">
                <h2>Manual Control</h2>
                <div class="control">
                    <button class="btn-on" onclick="controlDevice('pump', true)">Pump ON</button>
                    <button class="btn-off" onclick="controlDevice('pump', false)">Pump OFF</button>
                </div>
                <div class="control">
                    <button class="btn-on" onclick="controlDevice('fan', true)">Fan ON</button>
                    <button class="btn-off" onclick="controlDevice('fan', false)">Fan OFF</button>
                </div>
                <div class="control">
                    <button class="btn-on" onclick="controlDevice('heater', true)">Heater ON</button>
                    <button class="btn-off" onclick="controlDevice('heater', false)">Heater OFF</button>
                </div>
                <div class="control">
                    <button class="btn-on" onclick="controlDevice('light', true)">Light ON</button>
                    <button class="btn-off" onclick="controlDevice('light', false)">Light OFF</button>
                </div>
                <div class="control">
                    <button class="btn-neutral" onclick="controlDevice('door', 0)">Open Door</button>
                    <button class="btn-neutral" onclick="controlDevice('door', 90)">Close Door</button>
                </div>
            </div>
            
            <!-- Settings -->
            <div class="card">
                <h2>Settings</h2>
                <form id="settingsForm">
                    <div class="form-group">
                        <label>Temperature Setpoint (C):</label>
                        <input type="number" step="0.1" id="tempSetpoint" min="10" max="40" required>
                    </div>
                    <div class="form-group">
                        <label>Humidity Setpoint (%):</label>
                        <input type="number" step="0.1" id="humSetpoint" min="20" max="90" required>
                    </div>
                    <div class="form-group">
                        <label>Soil Moisture Setpoint (%):</label>
                        <input type="number" step="0.1" id="soilMoistureSetpoint" min="10" max="90" required>
                    </div>
                    <div class="form-group">
                        <label>Light ON Hour:</label>
                        <input type="number" id="lightOnHour" min="0" max="23" required>
                    </div>
                    <div class="form-group">
                        <label>Light OFF Hour:</label>
                        <input type="number" id="lightOffHour" min="0" max="23" required>
                    </div>
                    <div class="form-group">
                        <label><input type="checkbox" id="automationEnabled"> Automation Enabled</label>
                    </div>
                    <button type="submit">Save Settings</button>
                </form>
            </div>
            
            <!-- System Info -->
            <div class="card">
                <h2>System Information</h2>
                <div id="systemInfo">
                    <p>Loading...</p>
                </div>
                <div class="control">
                    <button class="btn-neutral" onclick="calibrate('air')">Calibrate Air</button>
                    <button class="btn-neutral" onclick="calibrate('water')">Calibrate Water</button>
                    <button class="btn-neutral" onclick="resetSettings('settings')">Reset Settings</button>
                </div>
            </div>
        </div>
    </div>

    <script>
        function refreshData() {
            console.log('Refreshing data...');
            fetch('/api/sensors')
                .then(function(r) { 
                    if (!r.ok) throw new Error('Network error: ' + r.status);
                    return r.json(); 
                })
                .then(function(data) {
                    console.log('Sensor data:', data);
                    var html = '';
                    if (data.airTemperature !== undefined) {
                        html += '<p>Air Temp: <span class="sensor-value">' + data.airTemperature.toFixed(1) + 'C</span></p>';
                    }
                    if (data.airHumidity !== undefined) {
                        html += '<p>Air Humidity: <span class="sensor-value">' + data.airHumidity.toFixed(1) + '%</span></p>';
                    }
                    if (data.soilTemperature !== undefined) {
                        html += '<p>Soil Temp: <span class="sensor-value">' + data.soilTemperature.toFixed(1) + 'C</span></p>';
                    }
                    if (data.soilMoisture !== undefined) {
                        html += '<p>Soil Moisture: <span class="sensor-value">' + data.soilMoisture.toFixed(1) + '%</span></p>';
                    }
                    if (data.lightLevel !== undefined) {
                        html += '<p>Light Level: <span class="sensor-value">' + data.lightLevel.toFixed(0) + ' lux</span></p>';
                    }
                    document.getElementById('sensorData').innerHTML = html;
                })
                .catch(function(error) {
                    console.error('Error:', error);
                    document.getElementById('sensorData').innerHTML = '<p style="color: red;">Error loading data: ' + error.message + '</p>';
                });
                
            fetch('/api/system')
                .then(function(r) { 
                    if (!r.ok) throw new Error('Network error: ' + r.status);
                    return r.json(); 
                })
                .then(function(data) {
                    var html = '<p>System: <span class="device-status ' + (data.systemHealthy ? 'healthy' : 'unhealthy') + '">' + (data.systemHealthy ? 'HEALTHY' : 'ERROR') + '</span></p>';
                    html += '<p>BME280: <span class="device-status ' + (data.bme280Healthy ? 'healthy' : 'unhealthy') + '">' + (data.bme280Healthy ? 'OK' : 'ERROR') + '</span></p>';
                    html += '<p>BH1750: <span class="device-status ' + (data.bh1750Healthy ? 'healthy' : 'unhealthy') + '">' + (data.bh1750Healthy ? 'OK' : 'ERROR') + '</span></p>';
                    html += '<p>Soil Sensors: <span class="device-status ' + (data.soilSensorsHealthy ? 'healthy' : 'unhealthy') + '">' + (data.soilSensorsHealthy ? 'OK' : 'ERROR') + '</span></p>';
                    document.getElementById('systemInfo').innerHTML = html;
                })
                .catch(function(error) {
                    console.error('Error:', error);
                    document.getElementById('systemInfo').innerHTML = '<p style="color: red;">Error loading system info</p>';
                });
        }
        
        function controlDevice(device, state) {
            console.log('Controlling:', device, state);
            fetch('/api/control', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({device: device, state: state})
            })
            .then(function(r) { 
                if (!r.ok) throw new Error('Network error: ' + r.status);
                return r.json(); 
            })
            .then(function(data) {
                alert(data.message);
                refreshData();
            })
            .catch(function(error) {
                console.error('Error:', error);
                alert('Control failed: ' + error.message);
            });
        }
        
        function calibrate(type) {
            fetch('/api/calibrate?type=' + type, {method: 'POST'})
                .then(function(r) { 
                    if (!r.ok) throw new Error('Network error: ' + r.status);
                    return r.json(); 
                })
                .then(function(data) { 
                    alert(data.message); 
                })
                .catch(function(error) {
                    alert('Calibration failed: ' + error.message);
                });
        }
        
        function resetSettings(type) {
            if (confirm('Are you sure you want to reset ' + type + '?')) {
                fetch('/api/reset?type=' + type, {method: 'POST'})
                    .then(function(r) { 
                        if (!r.ok) throw new Error('Network error: ' + r.status);
                        return r.json(); 
                    })
                    .then(function(data) { 
                        alert(data.message);
                        loadSettings();
                    })
                    .catch(function(error) {
                        alert('Reset failed: ' + error.message);
                    });
            }
        }
        
        function loadSettings() {
            fetch('/api/settings')
                .then(function(r) { 
                    if (!r.ok) throw new Error('Network error: ' + r.status);
                    return r.json(); 
                })
                .then(function(data) {
                    document.getElementById('tempSetpoint').value = data.tempSetpoint;
                    document.getElementById('humSetpoint').value = data.humSetpoint;
                    document.getElementById('soilMoistureSetpoint').value = data.soilMoistureSetpoint;
                    document.getElementById('lightOnHour').value = data.lightOnHour;
                    document.getElementById('lightOffHour').value = data.lightOffHour;
                    document.getElementById('automationEnabled').checked = data.automationEnabled;
                })
                .catch(function(error) {
                    console.error('Error loading settings:', error);
                });
        }
        
        document.getElementById('settingsForm').onsubmit = function(e) {
            e.preventDefault();
            var settings = {
                tempSetpoint: parseFloat(document.getElementById('tempSetpoint').value),
                humSetpoint: parseFloat(document.getElementById('humSetpoint').value),
                soilMoistureSetpoint: parseFloat(document.getElementById('soilMoistureSetpoint').value),
                lightOnHour: parseInt(document.getElementById('lightOnHour').value),
                lightOffHour: parseInt(document.getElementById('lightOffHour').value),
                automationEnabled: document.getElementById('automationEnabled').checked
            };
            
            fetch('/api/settings', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(settings)
            })
            .then(function(r) { 
                if (!r.ok) throw new Error('Network error: ' + r.status);
                return r.json(); 
            })
            .then(function(data) {
                alert(data.message);
            })
            .catch(function(error) {
                alert('Save failed: ' + error.message);
            });
        };
        
        // Auto-refresh every 10 seconds
        setInterval(refreshData, 10000);
        
        // Initial load
        refreshData();
        loadSettings();
        
        console.log('Smart Greenhouse interface loaded');
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
    if (device == "pump" || device == "fan" || device == "heater" || device == "light") {
        return true;
    }
    if (device == "door") {
        return (state == 0 || state == 90);
    }
    return false;
}

void WebInterface::applyControlCommand(const String& device, bool state) {
    if (device == "pump") {
        deviceManager.controlPump(state);
    } else if (device == "fan") {
        deviceManager.controlFan(state);
    } else if (device == "heater") {
        deviceManager.controlHeater(state);
    } else if (device == "light") {
        deviceManager.controlLight(state);
    } else if (device == "door") {
        deviceManager.controlDoor(state ? 90 : 0);
    }
}