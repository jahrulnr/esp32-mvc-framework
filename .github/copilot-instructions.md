# ESP32 MVC Framework - AI Coding Assistant Instructions

## Project Overview
This is a Laravel-inspired MVC framework for ESP32 microcontrollers using PlatformIO and Arduino framework. It provides web applications with authentication, role-based access control, and a CSV-based database system running on SPIFFS flash storage.

## Architecture Patterns

### MVC Structure
- **Controllers**: In `app/Controllers/` - inherit from `Controller` base class, return `Response` objects
- **Models**: In `app/Models/` - inherit from `Model` base class, use CSV storage via `CsvDatabase`
- **Views**: HTML templates in `data/views/` served via SPIFFS filesystem
- **Routes**: Defined in `app/Routes/routes.cpp` using fluent Router API

### Key Framework Components
- **Application**: Singleton pattern in `lib/MVCFramework/src/Core/Application.h` - boots framework, manages services
- **Router**: Express.js-style routing with middleware support, parameter extraction, named routes
- **ServiceContainer**: Dependency injection container with template-based binding
- **Request/Response**: HTTP abstractions wrapping ESPAsyncWebServer
- **Middleware**: Chainable middleware for auth, CORS, rate limiting (applied via `.middleware()`)

### Database System
- **CsvDatabase**: File-based database using CSV format stored in SPIFFS
- **Model**: Active Record pattern with dirty tracking, JSON serialization
- **User Model**: Role-based system (0=System, 1=Admin, 2=User) with permission methods

## Development Workflow

### Build Commands
```bash
# Build project
make build  # or pio run

# Upload to ESP32
make upload  # or pio run -t upload

# Monitor serial output
make monitor  # or pio device monitor

# Build filesystem and upload
make fs  # or pio run -t uploadfs
```

### Environment Configuration
- **platformio.ini**: Two environments - `nodemcu32s` and `esp32cam`
- **SPIFFS**: File system for serving static files and storing database
- **Board-specific**: Use `BOARD_HAS_PSRAM` flag for ESP32-CAM builds

### Route Registration Pattern
```cpp
// In app/Routes/routes.cpp
router->get("/users", [controller](Request& request) -> Response {
    return controller->index(request);
}).middleware("auth").name("users.index");

// Route groups with shared middleware
router->group("/api", [](Router& api) {
    api.middleware({"cors", "json", "ratelimit"});
    api->get("/users", handler);
});
```

### Model Usage Pattern
```cpp
// Create new model
User user;
user.setName("John Doe");
user.setEmail("john@example.com");
user.setRole(2); // User role
user.save();

// Query by email
User* found = User::findByEmail("john@example.com");

// Role-based permissions
if (user->canManageUsers()) {
    // Admin/System level actions
}
```

## Critical Integration Points

### Authentication Flow
- Bearer token authentication via `auth` middleware
- User roles stored as integers (0=System, 1=Admin, 2=User)
- Permission checks in User model: `canManageUsers()`, `canAccessAdmin()`
- Frontend stores tokens in localStorage, sends via Authorization header

### SPIFFS File Structure
- `data/views/*.html` - HTML templates
- `data/assets/` - CSS/JS static files
- `data/database/*.csv` - CSV database files
- `data/config.json` - Application configuration

### Controller Response Patterns
```cpp
// Return HTML view
return Response(request.getServerRequest()).html(htmlContent);

// Return JSON API response
return Response(request.getServerRequest()).json(jsonData);

// Redirect
return Response(request.getServerRequest()).redirect("/login");
```

### Middleware Registration
Middleware is registered in `Application::registerMiddleware()` and applied via:
- Individual routes: `.middleware("auth")`
- Route groups: `api.middleware({"cors", "json"})`
- Global middleware for all routes

### Memory Management
- Use smart pointers in framework core (`std::unique_ptr`, `std::shared_ptr`)
- Manual memory management in application layer (controllers allocated with `new`)
- ESP32-CAM requires PSRAM configuration for camera operations

## Project-Specific Conventions

### File Organization
- Main application in `app/app.ino` (entry point)
- Framework library in `lib/MVCFramework/src/`
- Include `MVCFramework.h` for framework access
- Controllers return `Response` objects, not direct HTTP responses

### Role-Based UI
- Dashboard content changes based on user role
- Admin features hidden from regular users
- Role checks both in backend (model permissions) and frontend (UI rendering)

### CSV Database Schema
- Tables defined as vectors of column names
- Primary key always "id" (auto-increment)
- Timestamps: "created_at", "updated_at" (string format)
- Boolean values stored as "0"/"1" strings
