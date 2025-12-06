#include "task_wifi.h"
#include "task_webserver.h" // Needed to verify server status

void startAP()
{
    Serial.println("⚠️ Wi-Fi Connection failed. Switching to AP Mode.");
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    // You can change "12345678" to your preferred default password
    WiFi.softAP("ESP32_Config_Mode", "12345678"); 
    Serial.print("📡 AP IP Address: ");
    Serial.println(WiFi.softAPIP());
}

void startSTA()
{
    if (WIFI_SSID.isEmpty())
    {
        Serial.println("No SSID stored. Starting AP.");
        startAP();
        return;
    }

    WiFi.mode(WIFI_STA);
    
    // Check if we are already connecting to avoid spamming begin()
    if(WiFi.status() == WL_CONNECTED) return;

    if (WIFI_PASS.isEmpty()) WiFi.begin(WIFI_SSID.c_str());
    else WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());

    Serial.print("Connecting to Wi-Fi");
    
    // TIMEOUT LOGIC: Try for 10 seconds (20 * 500ms), then give up
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        Serial.print(".");
        timeout++;
    }
    Serial.println();

    if(WiFi.status() == WL_CONNECTED) {
        Serial.print("✅ Connected! IP: ");
        Serial.println(WiFi.localIP());
        xSemaphoreGive(xBinarySemaphoreInternet);
    } else {
        // If we timed out, start AP mode so the user can access the Webserver
        startAP();
    }
}

bool Wifi_reconnect()
{
    // If we are in AP mode, we are technically "connected" to ourselves, 
    // but this function usually checks for Internet access.
    if (WiFi.status() == WL_CONNECTED)
    {
        return true;
    }
    
    // Only try to reconnect if we aren't already in AP mode
    if(WiFi.getMode() != WIFI_AP) {
        startSTA();
    }
    
    return (WiFi.status() == WL_CONNECTED);
}