#include "temp_humi_monitor.h"
#include "task_webserver.h"
DHT20 dht20;


void temp_humi_monitor(void *pvParameters){

    Wire.begin(11, 12);
    Serial.begin(115200);
    dht20.begin();

    while (1){
        /* code */
        dht20.read();
        SensorData currentreadings;
        // Reading temperature in Celsius
        currentreadings.temperature = dht20.getTemperature();
        
        // Reading humidity
        currentreadings.humidity = dht20.getHumidity();
        
        xQueueOverwrite(xQueueSensorData, &currentreadings);

        // Check if any reads failed and exit early
        if (isnan(currentreadings.temperature) || isnan(currentreadings.humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            currentreadings.temperature = currentreadings.humidity =  -1;
            //return;
        }
        //Determine system state based on temperature
        if(currentreadings.temperature < 28.0){
            //Normal State
            xSemaphoreGive(xSemStateNormal);
        }
        else if(currentreadings.temperature >= 28.0 && currentreadings.temperature < 32.0){
            //Warning State
            xSemaphoreGive(xSemStateWarning);
        }
        else{
            //Critical State
            xSemaphoreGive(xSemStateCritical);
        }
        // Print the results
        Serial.print("Humidity: ");
        Serial.print(currentreadings.humidity);
        Serial.print("%  Temperature: ");
        Serial.print(currentreadings.temperature);
        Serial.println("°C");
        Webserver_broadcastSensors(currentreadings.temperature, currentreadings.humidity);
        vTaskDelay(5000);
    }
    
}