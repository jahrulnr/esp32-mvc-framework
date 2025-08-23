#include <Arduino.h>
#include "app.h"
#include "Models/Configuration.h"
#include "Models/ServoConfig.h"

Application* app;
CsvDatabase* database;

// const char* ssid = "PioSystem";
// const char* password = "tes12345";
const char* ssid = "ANDROID AP";
const char* password = "tes12345";

String deviceName = "pio-esp32-cam";

void setup() {
    #ifdef BOARD_HAS_PSRAM
  	heap_caps_malloc_extmem_enable(4096);
    #endif

    Serial.begin(115200);
    Serial.println("Starting ESP32 MVC Application...");
    
    // Initialize application
    app = Application::getInstance();
    app->setDeviceName(deviceName.c_str());
    
    // Boot the framework
    app->boot();
    
    // Initialize CSV database first (needed for Configuration model)
    database = new CsvDatabase();
    Model::setDatabase(database);
    
    // Initialize database tables
    Configuration::initTable();
    ServoConfigModel::initTable();
    
    Serial.println("Database tables initialized");
    
    // Set hostname before connecting
    WiFi.setHostname(deviceName.c_str());
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    Serial.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Device hostname: ");
    Serial.println(WiFi.getHostname());
    
    // Set up mDNS responder for local name resolution
    if (MDNS.begin(deviceName.c_str())) {
        Serial.println("mDNS responder started: " + deviceName + ".local");
        // Add service to mDNS
        MDNS.addService("http", "tcp", 80);
    } else {
        Serial.println("Error setting up mDNS responder");
    }
    
    // Register routes
    Router* router = app->getRouter();
    registerWebSocketRoutes(router);
    registerWebRoutes(router);
    registerApiRoutes(router);
    
    // Start the application
    app->run();
    setupTasks();
    
    Serial.println("WiFi monitoring task created");
}

void loop() {
    ServoManager& servoManager = ServoManager::getInstance();

    // Main loop is now free for other tasks
    // WiFi monitoring is handled by FreeRTOS task
    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 second delay
}