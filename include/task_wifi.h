#ifndef TASK_WIFI_H
#define TASK_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include "global.h" // Imports WIFI_SSID, WIFI_PASS, and semaphores

// Define the functions so main.cpp can see them
void startAP();
void startSTA();
bool Wifi_reconnect();

#endif