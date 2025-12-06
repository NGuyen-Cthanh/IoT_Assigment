#include "neo_blinky.h"
#include "global.h"

// Define humidity thresholds
#define HUMI_LOW_THRESHOLD  40.0
#define HUMI_HIGH_THRESHOLD 70.0

void neo_blinky(void *pvParameters){
    // Initialize the NeoPixel strip
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.show(); // Turn off all pixels initially
    
    // Variable to store the color we want to display
    uint32_t color = strip.Color(0, 0, 0); // Default to off
    SensorData data;

    while(1) {
        // --- SEMAPHORE LOGIC START ---
        // 1. Acquire the Mutex to read shared data safely
        if (xQueuePeek(xQueueSensorData, &data,0) == pdTRUE) {
            
            // 2. Read the global humidity variable
            float humi = data.humidity;
            
            // 4. Determine Color Based on Humidity
            // We calculate the color *after* releasing the semaphore to keep the lock time short.
            if (humi < HUMI_LOW_THRESHOLD) {
                // Dry: Red
                color = strip.Color(255, 0, 0);
            } 
            else if (humi >= HUMI_LOW_THRESHOLD && humi <= HUMI_HIGH_THRESHOLD) {
                // Comfort: Green
                color = strip.Color(0, 255, 0);
            } 
            else {
                // Humid: Blue
                color = strip.Color(0, 0, 255);
            }
        }
        // Update the NeoPixel (Using Pixel 0 as the indicator)
        strip.setPixelColor(0, color);
        strip.show();

        // Update rate: 500ms
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}