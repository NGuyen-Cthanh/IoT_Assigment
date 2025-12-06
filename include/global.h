#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

// Sensor Data Struct
struct SensorData {
  float temperature;
  float humidity;
};

// RTOS Handles
extern QueueHandle_t xQueueSensorData;
extern SemaphoreHandle_t xSemStateNormal;
extern SemaphoreHandle_t xSemStateWarning;
extern SemaphoreHandle_t xSemStateCritical;

// WiFi & IoT Variables (These will be updated via Web Server)
extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;

#endif