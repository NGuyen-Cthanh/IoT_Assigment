#include "global.h"

// ============================================================
// RTOS HANDLES DEFINITIONS
// ============================================================
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

// Handles for Task 3 Logic
QueueHandle_t xQueueSensorData = NULL;
SemaphoreHandle_t xSemStateNormal = NULL;
SemaphoreHandle_t xSemStateWarning = NULL;
SemaphoreHandle_t xSemStateCritical = NULL;

// ============================================================
// WIFI / IOT SETTINGS
// ============================================================
String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

String ssid = "Tan Tien 70";
String password = "namphuong";
String wifi_ssid = "Tan Tien 70";
String wifi_password = "namphuong";
boolean isWifiConnected = false;