#include "task_webserver.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool webserver_isrunning = false;

// -------------------------------------------------------------------------
// 1. Broadcast Sensor Data to Dashboard
// -------------------------------------------------------------------------
void Webserver_broadcastSensors(float temp, float humi) {
    // Only send if there are connected clients to save resources
    if (ws.count() > 0) {
        DynamicJsonDocument doc(256);
        doc["temperature"] = temp;
        doc["humidity"] = humi;
        
        String output;
        serializeJson(doc, output);
        ws.textAll(output);
    }
}

// -------------------------------------------------------------------------
// 2. Handle Settings Save (Write to LittleFS)
// -------------------------------------------------------------------------
void saveConfig(JsonObject data) {
    Serial.println("💾 Saving Settings to /config.json...");

    // Update Global Variables immediately
    if (data.containsKey("ssid")) WIFI_SSID = data["ssid"].as<String>();
    if (data.containsKey("password")) WIFI_PASS = data["password"].as<String>();
    if (data.containsKey("token")) CORE_IOT_TOKEN = data["token"].as<String>();
    if (data.containsKey("server")) CORE_IOT_SERVER = data["server"].as<String>();
    if (data.containsKey("port")) CORE_IOT_PORT = data["port"].as<String>();

    // Save to File for persistence across reboots
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("❌ Failed to open config file for writing");
        return;
    }

    serializeJson(data, configFile);
    configFile.close();
    Serial.println("✅ Settings Saved!");
}

// -------------------------------------------------------------------------
// 3. Process Incoming JSON Commands
// -------------------------------------------------------------------------
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        
        data[len] = 0;
        String message = (char *)data;
        // Serial.print("📩 Received: "); Serial.println(message); // Debug

        // Allocate buffer (1024 bytes to handle larger settings JSON)
        DynamicJsonDocument doc(1024); 
        DeserializationError error = deserializeJson(doc, message);
        
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }

        // --- ROUTING LOGIC ---
        if (doc.containsKey("page")) {
            String page = doc["page"];
            JsonObject value = doc["value"];

            // A. DEVICE PAGE (Relay Control)
            if (page == "device") {
                int gpio = value["gpio"].as<int>();
                String status = value["status"].as<String>();
                
                // Configure pin dynamically
                pinMode(gpio, OUTPUT);
                
                if (status == "ON") {
                    digitalWrite(gpio, HIGH);
                    Serial.printf("⚡ GPIO %d turned ON\n", gpio);
                } else {
                    digitalWrite(gpio, LOW);
                    Serial.printf("⚪ GPIO %d turned OFF\n", gpio);
                }
            }
            
            // B. SETTINGS PAGE (WiFi & IoT)
            else if (page == "setting") {
                saveConfig(value);
            }
        }
    }
}

// -------------------------------------------------------------------------
// 4. WebSocket Boilerplate
// -------------------------------------------------------------------------
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WS Client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WS Client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

// -------------------------------------------------------------------------
// 5. Setup & Routing
// -------------------------------------------------------------------------
void Webserver_setup() {
    if (!LittleFS.begin()) {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    ws.onEvent(onEvent);
    server.addHandler(&ws);

    // --- HTML/CSS/JS Routes ---
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/styles.css", "text/css");
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/script.js", "application/javascript");
    });

    // --- LOCAL LIBRARY ROUTES (Fixes missing gauges) ---
    server.on("/raphael.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/raphael.min.js", "application/javascript");
    });
    server.on("/justgage.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/justgage.min.js", "application/javascript");
    });

    ElegantOTA.begin(&server);
    server.begin();
    webserver_isrunning = true;
    Serial.println("🚀 Webserver Started.");
}

void Webserver_stop() {
    ws.closeAll();
    server.end();
    webserver_isrunning = false;
}

void Webserver_reconnect() {
    if (!webserver_isrunning) {
        Webserver_setup();
    }
    ElegantOTA.loop();
}