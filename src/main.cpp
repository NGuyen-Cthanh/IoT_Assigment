#include "global.h"

// Include tasks
#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "tinyml.h"
#include "coreiot.h"
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task_core_iot.h"
#include "task_lcd.h"

// -------------------------------------------------------------------------
// NEW: System Maintenance Task (Replaces void loop)
// -------------------------------------------------------------------------
void task_system_manager(void *pvParameters)
{
  Serial.println("⚙️ System Manager Task Started");

  while (1)
  {
    // 1. Check for Config/Reset Triggers
    if (check_info_File(1))
    {
      // If Wi-Fi is lost and we aren't in AP mode, try to reconnect
      if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP)
      {
        if (!Wifi_reconnect())
        {
           // Optional: Stop server if offline, though usually not strictly necessary
           // Webserver_stop(); 
        }
      }
      else
      {
        // Maintain IoT Connection if Wi-Fi is good
        CORE_IOT_reconnect();
      }
    }

    // 2. Handle OTA (Over-the-Air updates)
    Webserver_reconnect(); 

    // 3. Yield to other tasks (CRITICAL in FreeRTOS loops)
    vTaskDelay(100 / portTICK_PERIOD_MS); 
  }
}

// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);

  // --- Create Queues & Semaphores ---
  xQueueSensorData = xQueueCreate(1, sizeof(SensorData));
  if(xQueueSensorData == NULL) Serial.println("Error creating queue");
  
  xSemStateNormal = xSemaphoreCreateBinary();
  xSemStateWarning = xSemaphoreCreateBinary();
  xSemStateCritical = xSemaphoreCreateBinary();
  
  // --- Initialize Settings ---
  check_info_File(0);

  // --- Start Hardware Tasks ---
  xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, NULL, 2, NULL);
  xTaskCreate(task_lcd, "Task LCD Display", 3072, NULL, 2, NULL);
  
  xTaskCreate(tiny_ml_task, "Tiny ML Task", 4096, NULL, 2, NULL);
  xTaskCreate(coreiot_task, "CoreIOT Task", 4096, NULL, 2, NULL);

  // --- Start Infrastructure ---
  // IMPORTANT: We start Wi-Fi and Webserver here so they run immediately
  Serial.println("🚀 Booting Infrastructure...");
  startSTA();        // Attempt Wi-Fi Connection
  Webserver_setup(); // Start Web Server

  // --- Start System Manager (The new "Loop") ---
  // We give it 4096 stack because OTA and JSON parsing can be memory hungry
  xTaskCreate(task_system_manager, "System Manager", 4096, NULL, 1, NULL);
}

// -------------------------------------------------------------------------
// Main Loop (Now Empty)
// -------------------------------------------------------------------------
void loop()
{
  // The Arduino loop is now just a wrapper. 
  // We can delete this task to free up roughly 1.5KB - 2KB of RAM.
  vTaskDelete(NULL); 
}