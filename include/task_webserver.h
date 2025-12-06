#ifndef __TASK_WEBSERVER_H__
#define __TASK_WEBSERVER_H__

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncTCP.h>
#include <ArduinoJson.h> // Ensure ArduinoJson library is installed
#include <ElegantOTA.h>
#include "global.h"      // Access to global variables like WIFI_SSID, xQueueSensorData

extern AsyncWebServer server;
extern AsyncWebSocket ws;

// --- Setup & Lifecycle ---
void Webserver_setup();
void Webserver_stop();
void Webserver_reconnect();

// --- Data Broadcasting ---
// Call this from your sensor task to update the dashboard
void Webserver_broadcastSensors(float temp, float humi);

// --- Internal Handlers ---
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void saveConfig(JsonObject data);

#endif