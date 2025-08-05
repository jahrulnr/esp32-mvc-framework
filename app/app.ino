#include <Arduino.h>
#include <WiFi.h>
#include <MVCFramework.h>
#include "Routes/routes.h"

// WiFi credentials
const char* ssid = "ANDROID AP";
const char* password = "tes12345";

Application* app;
CsvDatabase* database;

void setup() {
		#ifdef BOARD_HAS_PSRAM
  	heap_caps_malloc_extmem_enable(4096);
		#endif

    Serial.begin(115200);
    Serial.println("Starting ESP32 MVC Application...");
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    
    // Initialize CSV database
    database = new CsvDatabase();
    Model::setDatabase(database);
    
    // Create users table if it doesn't exist
    if (!database->tableExists("users")) {
        std::vector<String> columns = {"name", "email", "password", "active", "role", "created_at", "updated_at"};
        database->createTable("users", columns);
        
        // Insert demo users with role-based access
        std::map<String, String> systemUser;
        systemUser["name"] = "System User";
        systemUser["email"] = "system@example.com";
        systemUser["password"] = "system123";
        systemUser["active"] = "1";
        systemUser["role"] = "0"; // System role
        systemUser["created_at"] = "2025-07-03 09:00:00";
        systemUser["updated_at"] = "2025-07-03 09:00:00";
        database->insert("users", systemUser);
        
        std::map<String, String> adminUser;
        adminUser["name"] = "Admin User";
        adminUser["email"] = "admin@example.com";
        adminUser["password"] = "password";
        adminUser["active"] = "1";
        adminUser["role"] = "1"; // Admin role
        adminUser["created_at"] = "2025-07-03 10:00:00";
        adminUser["updated_at"] = "2025-07-03 10:00:00";
        database->insert("users", adminUser);
        
        std::map<String, String> regularUser;
        regularUser["name"] = "Regular User";
        regularUser["email"] = "user@example.com";
        regularUser["password"] = "123456";
        regularUser["active"] = "1";
        regularUser["role"] = "2"; // User role
        regularUser["created_at"] = "2025-07-03 10:01:00";
        regularUser["updated_at"] = "2025-07-03 10:01:00";
        database->insert("users", regularUser);
        
        Serial.println("CSV Database initialized with demo users");
    }
    
    // Initialize application
    app = Application::getInstance();
    
    // Boot the framework
    app->boot();
    
    // Register routes
    Router* router = app->getRouter();
    registerWebRoutes(router);
    registerApiRoutes(router);
    
    // Start the application
    app->run();
    
    Serial.println("ESP32 MVC Application is running!");
    Serial.println("Available routes:");
    Serial.println("Web Routes:");
    Serial.println("- GET  /              (Home page)");
    Serial.println("- GET  /about         (About page)");
    Serial.println("- GET  /contact       (Contact form)");
    Serial.println("- POST /contact       (Submit contact)");
    Serial.println("- GET  /login         (Login page)");
    Serial.println("- POST /login         (Process login)");
    Serial.println("- GET  /register      (Registration page)");
    Serial.println("- POST /register      (Process registration)");
    Serial.println("- GET  /dashboard     (Dashboard - requires auth)");
    Serial.println("- GET  /users         (User Management - admin/system only)");
    Serial.println("- GET  /csv-demo      (CSV Database Demo)");
    Serial.println("- POST /logout        (Logout)");
    Serial.println("API Routes:");
    Serial.println("- GET  /health        (Health check)");
    Serial.println("- GET  /api/v1/items  (API: List items)");
    Serial.println("- GET  /api/v1/items/{id} (API: Show item)");
    Serial.println("- POST /api/v1/items  (API: Create item)");
    Serial.println("- PUT  /api/v1/items/{id} (API: Update item)");
    Serial.println("- DELETE /api/v1/items/{id} (API: Delete item)");
    Serial.println("- GET  /api/v1/status (API: System status)");
    Serial.println("- GET  /api/v1/health (API: Health check)");
    Serial.println("- GET  /api/v1/version (API: Version info)");
    Serial.println("- GET  /api/v1/users  (API: Users from CSV database)");
    Serial.println("User Management API Routes (Role-based):");
    Serial.println("- GET  /api/v1/admin/users     (List users - admin/system)");
    Serial.println("- GET  /api/v1/admin/users/{id} (Show user - admin/system)");
    Serial.println("- POST /api/v1/admin/users     (Create user - admin/system)");
    Serial.println("- PUT  /api/v1/admin/users/{id} (Update user - admin/system)");
    Serial.println("- DELETE /api/v1/admin/users/{id} (Delete user - admin/system)");
    Serial.println("");
    Serial.println("Database: CSV files stored in SPIFFS (/data/database/)");
    Serial.println("Role-based Access Control:");
    Serial.println("- Role 0 (System): Full access to all users and system");
    Serial.println("- Role 1 (Admin): Can manage regular users (role 2)");
    Serial.println("- Role 2 (User): Limited access to own profile only");
    Serial.println("");
    Serial.println("Authentication:");
    Serial.println("Demo credentials - system@example.com:system123 (System)");
    Serial.println("                 - admin@example.com:password (Admin)");
    Serial.println("                 - user@example.com:123456 (User)");
    Serial.println("(User data loaded from CSV database with role-based permissions)");
}

void loop() {
    // Main loop - the web server handles requests asynchronously
    delay(1000);
    
    // You can add periodic tasks here
    // For example: sensor readings, status updates, etc.
    
    // Print memory usage every 30 seconds
    static unsigned long lastMemoryCheck = 0;
    if (millis() - lastMemoryCheck > 30000) {
        Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        lastMemoryCheck = millis();
    }
}