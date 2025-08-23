# ESP32 MVC Framework

A comprehensive Model-View-Controller (MVC) framework for ESP32 microcontrollers that provides web server capabilities, authentication, routing, database management, and real-time communication through WebSockets.

## 🚀 Features

### Core Framework
- **MVC Architecture**: Clean separation of concerns with Models, Views, and Controllers
- **Routing System**: Flexible HTTP routing with support for route groups, middleware, and named routes
- **Middleware Support**: Built-in middleware for authentication, CORS, JSON handling, and rate limiting
- **Service Container**: Dependency injection container for managing application services
- **Configuration Management**: Environment-based configuration system

### Web Server & Communication
- **Async Web Server**: Built on ESPAsyncWebServer for high-performance HTTP handling
- **WebSocket Support**: Real-time bidirectional communication
- **SPIFFS Integration**: File system support for serving static assets and templates
- **RESTful API**: JSON-based API endpoints with proper HTTP status codes
- **Single Page Application**: Support for modern web applications

### Database & Models
- **CSV Database**: Lightweight file-based database system using CSV format
- **ORM-like Models**: Active Record pattern with relationships and validations
- **Data Persistence**: Configuration and user data persistence across reboots
- **Migration Support**: Database schema management and updates

### Authentication & Security
- **Token-based Authentication**: Session management with secure tokens
- **Role-based Access Control**: Admin and user role management
- **Input Validation**: Request validation and sanitization
- **CORS Support**: Cross-origin resource sharing configuration

### Hardware Integration
- **Camera Support**: AI-Thinker ESP32-CAM integration with streaming capabilities
- **Servo Control**: Multi-servo management with preset configurations
- **System Monitoring**: Real-time system stats, memory usage, and network information
- **WiFi Management**: Auto-reconnection and network monitoring

## 📋 Requirements

### Hardware
- ESP32 development board (NodeMCU-32S, ESP32-CAM, etc.)
- Minimum 4MB flash memory
- WiFi connectivity

### Software
- PlatformIO IDE
- Arduino Framework for ESP32
- SPIFFS filesystem support

### Dependencies
- `ESP32Async/ESPAsyncWebServer` (^3.7.10)
- `bblanchon/ArduinoJson` (^7.0.0)
- `rweather/Crypto` (^0.4.0)
- `madhephaestus/ESP32Servo` (^3.0.6) - for servo control

## 🛠️ Installation

### Method 1: PlatformIO Library Manager
```bash
pio lib install https://github.com/jahrulnr/ESP32-MVC-Framework.git
```

### Method 2: Git Clone
```bash
git clone https://github.com/jahrulnr/ESP32-MVC-Framework.git
cd ESP32-MVC-Framework
```

### Method 3: Manual Download
Download the latest release from GitHub and extract to your PlatformIO `lib` directory.

## 🏃‍♂️ Quick Start

### 1. Basic Setup

Create a new PlatformIO project and include the framework:

```cpp
#include <Arduino.h>
#include <MVCFramework.h>

Application* app;

void setup() {
    Serial.begin(115200);
    
    // Initialize application
    app = Application::getInstance();
    app->boot();
    
    // Connect to WiFi
    WiFi.begin("your_ssid", "your_password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    // Start the application
    app->run();
}

void loop() {
    // Application loop handled by framework
}
```

### 2. Define Routes

Create a routes file to define your application endpoints:

```cpp
#include <MVCFramework.h>

void registerRoutes(Router* router) {
    // Basic route
    router->get("/", [](Request& request) -> Response {
        return Response(request.getServerRequest())
            .json("{\"message\": \"Hello World!\"}");
    });
    
    // API routes with middleware
    router->group("/api/v1", [&](Router& api) {
        api.middleware({"auth", "json"});
        
        api.get("/status", [](Request& request) -> Response {
            JsonDocument doc;
            doc["status"] = "ok";
            doc["uptime"] = millis();
            return Response(request.getServerRequest()).json(doc);
        });
    });
}
```

### 3. Create Controllers

```cpp
#include <MVCFramework.h>

class UserController : public Controller {
public:
    Response index(Request& request) {
        JsonDocument users;
        users["users"] = JsonArray();
        return json(request.getServerRequest(), users);
    }
    
    Response show(Request& request) {
        String id = request.route("id");
        // Fetch user by ID
        return json(request.getServerRequest(), user_data);
    }
};
```

### 4. Define Models

```cpp
#include <MVCFramework.h>

class User : public Model {
public:
    User() : Model("users") {}
    
    // Model attributes
    String username;
    String email;
    String role;
    
    // Static methods for queries
    static std::vector<User*> all() {
        return Model::all<User>();
    }
    
    static User* find(const String& id) {
        return Model::find<User>(id);
    }
};
```

## 📖 Documentation

### Core Components

#### Application Class
The main application class that bootstraps the framework:

```cpp
Application* app = Application::getInstance();
app->boot();        // Initialize core services
app->run();         // Start the web server
```

#### Router
Define HTTP routes and route groups:

```cpp
// Basic routes
router->get("/users", handler);
router->post("/users", handler);
router->put("/users/{id}", handler);
router->delete_("/users/{id}", handler);

// Route groups with middleware
router->group("/api", [&](Router& api) {
    api.middleware({"auth", "cors"});
    // Define routes within group
});

// WebSocket routes
router->websocket("/ws/chat")
    .onConnect(onConnect)
    .onMessage(onMessage)
    .onDisconnect(onDisconnect);
```

#### Controllers
Handle HTTP requests and return responses:

```cpp
class MyController : public Controller {
public:
    Response index(Request& request) {
        return view(request, "index", data);
    }
    
    Response store(Request& request) {
        // Validate input
        if (!validate(request, {"name": "required", "email": "email"})) {
            return error(request, "Validation failed", 422);
        }
        
        // Process and return response
        return success(request, "Created successfully");
    }
};
```

#### Models
Interact with the database:

```cpp
class Product : public Model {
public:
    Product() : Model("products") {}
    
    String name;
    float price;
    int stock;
    
    bool save() {
        JsonDocument data;
        data["name"] = name;
        data["price"] = price;
        data["stock"] = stock;
        return Model::save(data);
    }
    
    static std::vector<Product*> inStock() {
        return where("stock", ">", "0");
    }
};
```

#### Middleware
Process requests before they reach controllers:

```cpp
class AuthMiddleware : public Middleware {
public:
    bool handle(Request& request, Response& response) override {
        String token = request.header("Authorization");
        if (!validateToken(token)) {
            response.status(401).json("{\"error\": \"Unauthorized\"}");
            return false; // Stop request processing
        }
        return true; // Continue to next middleware/controller
    }
};
```

### Configuration

#### Environment Configuration
Set configuration values:

```cpp
Config* config = app->getConfig();
config->set("app.name", "My ESP32 App");
config->set("server.port", "8080");
config->set("database.path", "/data");

// Get configuration values
String appName = config->get("app.name");
int port = config->getInt("server.port");
bool debug = config->getBool("app.debug");
```

#### WiFi Configuration
```cpp
// Basic WiFi setup
WiFi.begin("SSID", "PASSWORD");
WiFi.setHostname("esp32-device");

// Auto-reconnection
WiFi.setAutoReconnect(true);
WiFi.persistent(true);
```

### API Endpoints

The framework provides several built-in API endpoints:

#### Authentication
- `POST /login` - User authentication
- `POST /logout` - User logout
- `GET /api/v1/auth/user` - Get current user info

#### System Management
- `GET /api/v1/system/stats` - System statistics
- `GET /api/v1/system/memory` - Memory information
- `GET /api/v1/system/network` - Network status
- `POST /api/v1/system/restart` - Restart device

#### Camera (ESP32-CAM)
- `GET /api/v1/camera/settings` - Camera configuration
- `POST /api/v1/camera/settings` - Update camera settings
- `POST /api/v1/camera/capture` - Capture image
- `GET /api/v1/camera/status` - Camera status

#### Servo Control
- `GET /api/v1/servo/list` - List all servos
- `POST /api/v1/servo/angle` - Set servo angle
- `POST /api/v1/servo/preset/save` - Save servo preset
- `POST /api/v1/servo/preset/load` - Load servo preset

### WebSocket Communication

```cpp
// Camera streaming
ws://device-ip/ws/camera

// Send commands
{"command": "capture"}
{"command": "ping"}

// Receive responses
{"type": "welcome", "message": "Connected"}
{"type": "pong", "timestamp": 123456}
{"type": "error", "message": "Error description"}
```

## 🎯 Examples

### Complete Dashboard Application

The framework includes a complete example application in the `examples/dashboard` directory featuring:

- **Web Dashboard**: Modern responsive UI with Bootstrap
- **Authentication System**: Login/logout with session management
- **Camera Interface**: Live camera streaming and controls
- **Servo Control Panel**: Multi-servo management with presets
- **System Monitoring**: Real-time system statistics
- **Configuration Management**: Device settings and preferences

### Key Files:
- `examples/dashboard/app/app.ino` - Main application
- `examples/dashboard/app/Routes/routes.cpp` - Route definitions
- `examples/dashboard/app/Controllers/` - Controller implementations
- `examples/dashboard/data/views/app.html` - Frontend application

### Running the Example:
```bash
cd examples/dashboard
pio run --target upload
pio device monitor
```

## 🔧 Configuration Files

### platformio.ini
```ini
[env:esp32cam]
platform = espressif32
board = esp32cam
framework = arduino
lib_deps = 
    ESP32Async/ESPAsyncWebServer@^3.6.10
    bblanchon/ArduinoJson@^7.4.1
    ESP32-MVC-Framework
build_flags = 
    -DCAMERA_MODEL_AI_THINKER
    -DBOARD_HAS_PSRAM
monitor_speed = 115200
board_build.filesystem = spiffs
```

### Data Directory Structure
```
data/
├── config.json          # Application configuration
├── views/
│   └── app.html         # Main application template
├── assets/
│   ├── app.css         # Stylesheets
│   ├── app.js          # JavaScript
│   └── bootstrap.*     # Framework assets
└── database/
    ├── users.csv       # User data
    └── configurations.csv # App configurations
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines
- Follow Arduino/ESP32 coding standards
- Add tests for new features
- Update documentation
- Ensure backward compatibility

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- ESP32 Arduino Core team
- ESPAsyncWebServer library contributors
- ArduinoJson library by Benoit Blanchon
- Bootstrap framework for UI components

## 📞 Support

- **GitHub Issues**: [Report bugs and request features](https://github.com/jahrulnr/ESP32-MVC-Framework/issues)
- **Documentation**: Check the [examples](examples/) directory

## 🚧 Roadmap

- [ ] Database migrations system
- [ ] Template engine improvements
- [ ] Real-time notifications
- [ ] Enhanced security features
- [ ] Mobile app integration
- [ ] Cloud synchronization
- [ ] Plugin system
- [ ] Advanced caching mechanisms

---
