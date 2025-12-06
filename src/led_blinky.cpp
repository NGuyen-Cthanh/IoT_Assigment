#include "led_blinky.h"
#include "global.h"

// Define blink delays (in milliseconds)
#define DELAY_NORMAL   1000
#define DELAY_WARNING  500
#define DELAY_CRITICAL 100

void led_blinky(void *pvParameters){
  // Ensure the LED pin is set to output
  // On Yolo Uno, verify if LED_GPIO is defined in your headers or use the specific pin number (often D13/GPIO13)
  pinMode(LED_GPIO, OUTPUT);
  
  int current_delay = DELAY_NORMAL;

  while(1) {
    SensorData data;
    if(xQueuePeek(xQueueSensorData, &data, portMAX_DELAY) == pdTRUE) {
    // We have successfully peeked at the latest sensor data
    float temp = data.temperature;
    // Determine blink speed based on temperature
      if (temp < 27.0) {
        current_delay = DELAY_NORMAL;     // Cool: Slow blink
      } else if (temp >= 27.0 && temp < 30.0) {
        current_delay = DELAY_WARNING;    // Warm: Medium blink
      } else {
        current_delay = DELAY_CRITICAL;   // Hot: Fast blink
      }
  }
    
  // Perform the blink with the calculated delay
  digitalWrite(LED_GPIO, HIGH);  
  vTaskDelay(current_delay / portTICK_PERIOD_MS);
    
  digitalWrite(LED_GPIO, LOW); 
  vTaskDelay(current_delay / portTICK_PERIOD_MS);
  }
}