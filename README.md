# ESP32 MVC Framework

A Laravel-inspired MVC framework for ESP32 microcontrollers, providing a structured approach to building web applications on embedded systems.

## Features

### Core Framework Components

- **MVC Architecture**: Clean separation of concerns with Models, Views, and Controllers
- **Routing System**: Express.js/Laravel-style routing with parameter support
- **Middleware Support**: Chainable middleware for authentication, CORS, rate limiting, etc.
- **Service Container**: Dependency injection container for managing services
- **Configuration Management**: JSON-based configuration with environment support
- **Request/Response Objects**: Rich HTTP request/response handling

### Built-in Middleware

- **CORS**: Cross-Origin Resource Sharing support
- **Authentication**: Bearer token authentication
- **Rate Limiting**: IP-based request rate limiting
- **Logging**: Request/response logging
- **JSON**: Automatic JSON content type handling

### Database System

- **CSV Database**: Lightweight file-based database using CSV format
- **SPIFFS Integration**: Data stored in ESP32 flash memory via SPIFFS
- **Model Integration**: Active Record pattern with CSV backend
- **CRUD Operations**: Full Create, Read, Update, Delete support
- **Query Builder**: Simple querying with where conditions

### Model System

- **Active Record Pattern**: Laravel-style model implementation
- **Attribute Management**: Dirty tracking and change detection
- **Validation**: Built-in validation with error reporting
- **Serialization**: JSON serialization support

## Directory Structure

```
app/
├── Framework/           # Core framework files
│   ├── Core/           # Application core (Application, Config, ServiceContainer)
│   ├── Http/           # HTTP components (Request, Response, Controller, Middleware)
│   ├── Routing/        # Router implementation
│   ├── Database/       # Model base classes
│   └── View/           # View rendering system
├── Controllers/        # Application controllers
├── Models/            # Application models
├── routes.cpp         # Route definitions
└── app.ino           # Main application file
```

## Getting Started

### 1. WiFi Configuration

Update the WiFi credentials in `app.ino`:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

### 2. Build and Upload

```bash
# Build the project
pio run

# Upload to ESP32
pio run -t upload

# Monitor serial output
pio device monitor
```

### 3. API Endpoints

Once running, the following endpoints will be available:

#### Web Routes
- `GET /` - Home page
- `GET /about` - About page  
- `GET /contact` - Contact form
- `POST /contact` - Submit contact form

#### API Routes (with CORS, JSON, Rate Limiting middleware)
- `GET /api/v1/items` - List all items
- `GET /api/v1/items/{id}` - Get specific item
- `POST /api/v1/items` - Create new item
- `PUT /api/v1/items/{id}` - Update item
- `DELETE /api/v1/items/{id}` - Delete item
- `GET /api/v1/users` - List users from CSV database
- `GET /api/v1/status` - System status
- `GET /api/v1/health` - Health check
- `GET /api/v1/version` - Version information

#### Authentication Routes
- `GET /login` - Login page
- `POST /login` - Process login
- `GET /register` - Registration page  
- `POST /register` - Process registration
- `GET /dashboard` - Protected dashboard
- `POST /logout` - Logout

#### System Routes
- `GET /health` - Framework health check
- `GET /csv-demo` - CSV Database demonstration page

## Usage Examples

### Creating Controllers

```cpp
#include "Framework/Http/Controller.h"

class MyController : public Controller {
public:
    Response index(Request& request) {
        JsonDocument data;
        data["message"] = "Hello from MyController!";
        return Response(request.getServerRequest()).json(data);
    }
    
    Response store(Request& request) {
        String name = request.input("name");
        if (name.length() == 0) {
            return error(request.getServerRequest(), "Name is required", 400);
        }
        
        // Process the data...
        return success(request.getServerRequest(), "Data saved successfully");
    }
};
```

### Creating Models

```cpp
#include "Framework/Database/Model.h"

DECLARE_MODEL(Product, "products")

class Product : public Model {
public:
    Product() : Model("products") {}
    
    String getName() const { return get("name"); }
    void setName(const String& name) { set("name", name); }
    
    float getPrice() const { return get("price").toFloat(); }
    void setPrice(float price) { set("price", String(price)); }
    
    bool validate() const {
        return getName().length() > 0 && getPrice() > 0;
    }
};
```

### Defining Routes

```cpp
void registerRoutes(Router* router) {
    MyController* controller = new MyController();
    
    // Simple routes
    router->get("/products", [controller](Request& request) -> Response {
        return controller->index(request);
    });
    
    // Routes with middleware
    router->group("/api", [&](Router& api) {
        api.middleware({"cors", "json", "auth"});
        
        api.post("/products", [controller](Request& request) -> Response {
            return controller->store(request);
        });
    });
}
```

### Using Middleware

```cpp
// Custom middleware
class CustomMiddleware : public Middleware {
public:
    Response handle(Request& request, std::function<Response(Request&)> next) override {
        // Pre-processing
        Serial.println("Before request");
        
        // Continue to next middleware/handler
        Response response = next(request);
        
        // Post-processing
        Serial.println("After request");
        
        return response;
    }
};

// Register middleware
router->registerMiddleware("custom", std::make_shared<CustomMiddleware>());
```

## Configuration

The framework uses JSON configuration stored in `/data/config.json`. Key settings include:

- **app.name**: Application name
- **app.env**: Environment (development/production)
- **app.debug**: Debug mode flag
- **server.port**: HTTP server port
- **features**: Enable/disable framework features

## Dependencies

The framework requires the following libraries (automatically managed by PlatformIO):

- `ESPAsyncWebServer` - Async HTTP server
- `AsyncTCP` - Async TCP library
- `ArduinoJson` - JSON parsing and generation

## Memory Usage

The framework is designed to be memory-efficient for ESP32 constraints:

- Minimal heap allocations
- Efficient string handling
- Optional features can be disabled
- Built-in memory monitoring

## Contributing

To extend the framework:

1. Follow the existing code structure
2. Add new middleware in `Framework/Http/`
3. Extend base classes for custom functionality
4. Update documentation for new features

## License

This project is open source and available under the MIT License.

## CSV Database Usage

The framework includes a lightweight CSV-based database system perfect for ESP32 applications.

### Setting Up the Database

```cpp
// Initialize CSV database
CsvDatabase* database = new CsvDatabase();
Model::setDatabase(database);

// Create a table
std::vector<String> columns = {"name", "email", "password", "active"};
database->createTable("users", columns);
```

### CSV File Format

Data is stored in SPIFFS under `/database/` directory:

```csv
id,name,email,password,active,role,created_at,updated_at
1,Admin User,admin@example.com,password,1,admin,2025-07-03 10:00:00,2025-07-03 10:00:00
2,Regular User,user@example.com,123456,1,user,2025-07-03 10:01:00,2025-07-03 10:01:00
```

### Using Models with CSV Database

```cpp
// Create new user
User* user = new User();
user->setName("John Doe");
user->setEmail("john@example.com");
user->setPassword("password123");
user->setActive(true);
user->save(); // Saves to CSV file

// Find user by email
User* user = User::findByEmail("john@example.com");

// Get all active users
std::vector<User*> users = User::active();

// Update user
user->setName("John Smith");
user->save();

// Delete user
user->delete_();
```

### Database Operations

```cpp
// Direct database operations
CsvDatabase* db = Model::getDatabase();

// Insert record
std::map<String, String> data;
data["name"] = "New User";
data["email"] = "new@example.com";
db->insert("users", data);

// Query with conditions
std::map<String, String> where;
where["active"] = "1";
auto records = db->select("users", where);

// Update record
std::map<String, String> updates;
updates["name"] = "Updated Name";
db->update("users", "1", updates);
```

### Authentication with CSV Database

The framework provides authentication using the CSV database:

- **Login**: Validates credentials against user records in CSV
- **Registration**: Creates new user records in CSV
- **User Management**: CRUD operations for user accounts

Demo credentials (stored in CSV):
- `admin@example.com` / `password`
- `user@example.com` / `123456`

Visit `/csv-demo` to see the CSV database in action!

### Using Middleware
