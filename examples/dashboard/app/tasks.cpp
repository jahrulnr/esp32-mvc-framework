#include "app.h"

// FreeRTOS task handle for WiFi monitoring
TaskHandle_t wifiTaskHandle = NULL;

// FreeRTOS task for WiFi monitoring and reconnection
void wifiMonitorTask(void* parameter) {
    for (;;) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected! Attempting to reconnect...");
            
            // Try to reconnect
            WiFi.disconnect();
            vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 second delay
            
            WiFi.setHostname(deviceName.c_str());
            WiFi.begin(ssid, password);
            WiFi.setAutoReconnect(true);
            WiFi.persistent(true);
            
            // Wait up to 15 seconds for connection
            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 30) {
                vTaskDelay(500 / portTICK_PERIOD_MS); // 500ms delay
                Serial.print(".");
                attempts++;
            }
            
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println();
                Serial.print("WiFi reconnected! IP address: ");
                Serial.println(WiFi.localIP());
                
                // Restart mDNS after reconnection
                MDNS.end();
                if (MDNS.begin(deviceName.c_str())) {
                    Serial.println("mDNS responder restarted: " + deviceName + ".local");
                    MDNS.addService("http", "tcp", 80);
                }
            } else {
                Serial.println();
                Serial.println("Failed to reconnect to WiFi. Will try again in 10 seconds.");
            }
        }
        
        // Check WiFi status every 10 seconds
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void setupTasks() {
    // Create FreeRTOS task for WiFi monitoring
    xTaskCreate(
        wifiMonitorTask,    // Task function
        "WiFiMonitor",      // Task name
        4096,               // Stack size (4KB)
        NULL,               // Task parameter
        1,                  // Task priority (1 = low priority)
        &wifiTaskHandle     // Task handle
    );
}