#include "coreiot.h"

// ----------- CONFIGURE THESE! (UNTOUCHED) -----------
const char* coreIOT_Server = "10.235.76.226";  
const char* coreIOT_Token = "g7drm1amhd3dchr379xu";   // Device Access Token
const int   mqttPort = 1883;
// ----------------------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);
static SensorData mydata;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // IMPORTANT: CoreIOT requires the Token to be passed as the USERNAME field.
    // client.connect(clientID, username, password)
    if (client.connect(clientId.c_str(), coreIOT_Token, NULL)) {
        
      Serial.println("connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* method = doc["method"];
  if (strcmp(method, "setStateLED") == 0) {
    const char* params = doc["params"];
    if (strcmp(params, "ON") == 0) {
      Serial.println("Device turned ON.");
      // Implementation for turning LED ON can go here
    } else {   
      Serial.println("Device turned OFF.");
      // Implementation for turning LED OFF can go here
    }
  } else {
    Serial.print("Unknown method: ");
    Serial.println(method);
  }
}

void setup_coreiot(){

  // Wait for Internet connection from task_wifi or similar
  while(1){
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
      xSemaphoreGive(xBinarySemaphoreInternet); // Give it back so others can check
      break;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println(" Internet Connected! Setting up MQTT...");

  // Use the specific variables requested in the prompt
  client.setServer(coreIOT_Server, mqttPort);
  client.setCallback(callback);
}

void coreiot_task(void *pvParameters){
    Serial.println("🚀 Starting CoreIOT Task...");
    setup_coreiot();

    while(1){

        if (!client.connected()) {
            reconnect();
        }
        client.loop();

        // --- NEW: Read real data from the Queue ---
        // We use xQueuePeek to read the data without removing it, 
        // in case the LCD task or Webserver also needs to read it.
        if (xQueuePeek(xQueueSensorData, &mydata, 0) == pdTRUE) {
            
            // Create JSON Payload
            // Format: {"temperature": 25.5, "humidity": 60.2}
            String payload = "{\"temperature\":" + String(mydata.temperature) +  ",\"humidity\":" + String(mydata.humidity) + "}";
            
            // Publish to CoreIOT Telemetry topic
            client.publish("v1/devices/me/telemetry", payload.c_str());
            
            Serial.println("Published to CoreIOT: " + payload);
        } else {
            Serial.println("Queue empty: Waiting for Sensor Data...");
        }

        vTaskDelay(10000 / portTICK_PERIOD_MS);  // Publish every 10 seconds
    }
}