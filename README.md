# ESP32 MVC Framework

A Laravel-inspired MVC framework for ESP32 microcontrollers with authentication, user management, and role-based access control. This framework provides a structured approach to building web applications on embedded systems with modern web UI and RESTful APIs.

## Features

### Core Framework Components

- **MVC Architecture**: Clean separation of concerns with Models, Views, and Controllers
- **Routing System**: Express.js/Laravel-style routing with parameter support and pattern matching
- **Middleware Support**: Chainable middleware for authentication, CORS, rate limiting, logging
- **Service Container**: Dependency injection container for managing services
- **Configuration Management**: JSON-based configuration with environment support
- **Request/Response Objects**: Rich HTTP request/response handling with file serving support

### Authentication & Authorization

- **User Authentication**: Login/Register system with token-based sessions
- **Role-Based Access Control**: Three-tier role system (System=0, Admin=1, User=2)
- **Session Management**: Persistent login with localStorage token storage
- **Protected Routes**: Middleware-based route protection
- **Permission System**: Granular permissions for user management operations

### Database System

- **CSV Database**: Lightweight file-based database using CSV format
- **SPIFFS Integration**: Data stored in ESP32 flash memory via SPIFFS
- **Model Integration**: Active Record pattern with CSV backend
- **CRUD Operations**: Full Create, Read, Update, Delete support
- **User Management**: Complete user CRUD with role-based permissions
- **Query Builder**: Simple querying with where conditions and filtering

### Modern Web Interface

- **Responsive Design**: Modern, mobile-friendly user interface
- **Real-time Dashboard**: Live system statistics and logs
- **Role-based Navigation**: Dynamic UI based on user permissions
- **AJAX Integration**: Seamless API interactions without page reloads
- **Asset Management**: CSS/JS file serving with proper MIME types

### Built-in Middleware

- **CORS**: Cross-Origin Resource Sharing support
- **Authentication**: Bearer token authentication with role checking
- **Rate Limiting**: IP-based request rate limiting
- **Logging**: Request/response logging with debug output
- **JSON**: Automatic JSON content type handling

### User Role System

- **System Role (0)**: Full system access, can manage all users and system settings
- **Admin Role (1)**: Can manage regular users, access admin features
- **User Role (2)**: Basic access, can view own profile and standard features
- **Permission Inheritance**: Higher roles inherit lower role permissions
- **Dynamic UI**: Interface adapts based on user role and permissions

### Database System

- **CSV Database**: Lightweight file-based database using CSV format
- **SPIFFS Integration**: Data stored in ESP32 flash memory via SPIFFS
- **Model Integration**: Active Record pattern with CSV backend
- **CRUD Operations**: Full Create, Read, Update, Delete support
- **User Database**: Pre-populated with demo users for each role
- **Query Builder**: Simple querying with where conditions and email lookups
- **Data Persistence**: Automatic saving and loading from CSV files

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
│   ├── Routing/        # Router implementation with pattern matching
│   ├── Database/       # Model base classes and CSV database
│   └── View/           # View rendering system
├── Controllers/        # Application controllers
│   ├── HomeController.h/.cpp      # Home page controller
│   ├── AuthController.h/.cpp      # Authentication controller
│   ├── UserController.h/.cpp      # User management controller
│   └── ApiController.h/.cpp       # API endpoints controller
├── Models/             # Application models
│   └── User.h/.cpp     # User model with role-based permissions
├── routes/             # Route definitions (web and API)
│   └── routes.h/.cpp   # Route registration and handlers
└── app.ino             # Main application file
data/
├── views/              # HTML templates served via SPIFFS
│   ├── index.html      # Landing page
│   ├── login.html      # Login form
│   ├── register.html   # Registration form
│   ├── dashboard.html  # Protected dashboard
│   ├── users.html      # User listing page
│   ├── user-management.html  # User CRUD interface
│   └── csv-demo.html   # CSV database demo
├── assets/             # Static assets (CSS, JS, images)
│   ├── main.css        # Main stylesheet
│   └── main.js         # JavaScript framework
├── database/           # Database files
│   └── users.csv       # User data with roles
├── config.json         # Application configuration
└── favicon.ico         # Site favicon
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

### 3. Demo Users

The system comes with pre-configured demo users for testing:

| Email | Password | Role | Description |
|-------|----------|------|-------------|
| `system@example.com` | `system123` | System (0) | Full system access |
| `admin@example.com` | `password` | Admin (1) | User management access |
| `user@example.com` | `123456` | User (2) | Basic user access |
| `test@example.com` | `test123` | User (2) | Inactive test account |

### 4. Available Endpoints

Once running, the following endpoints will be available:

#### Web Routes
- `GET /` - Landing page with framework info
- `GET /about` - About page  
- `GET /contact` - Contact form
- `POST /contact` - Submit contact form
- `GET /login` - Login page
- `POST /login` - Process login (returns token)
- `GET /register` - Registration page  
- `POST /register` - Process registration
- `GET /dashboard` - Protected dashboard (requires auth)
- `GET /users` - User management page (Admin/System only)
- `GET /user-management` - Advanced user CRUD interface
- `GET /csv-demo` - CSV Database demonstration
- `POST /logout` - Logout and clear session
- `GET /assets/{file}` - Static asset serving (CSS, JS, images)
- `GET /favicon.ico` - Site favicon

#### Authentication API
- `GET /api/v1/auth/user` - Get current user info (requires Bearer token)

#### User Management API (Admin/System only)
- `GET /api/v1/admin/users` - List all users (role-filtered)
- `GET /api/v1/admin/users/{id}` - Get specific user
- `POST /api/v1/admin/users` - Create new user
- `PUT /api/v1/admin/users/{id}` - Update user
- `DELETE /api/v1/admin/users/{id}` - Delete user

#### System API Routes (with CORS, JSON, Rate Limiting middleware)
- `GET /api/v1/items` - List all items (demo)
- `GET /api/v1/items/{id}` - Get specific item
- `POST /api/v1/items` - Create new item
- `PUT /api/v1/items/{id}` - Update item
- `DELETE /api/v1/items/{id}` - Delete item
- `GET /api/v1/users` - List users from CSV database
- `GET /api/v1/status` - System status and statistics
- `GET /api/v1/health` - Health check
- `GET /api/v1/version` - Version information
- `POST /api/v1/restart` - Restart system (System role only)

## Usage Examples

### Authentication Flow

```javascript
// Login via JavaScript
const response = await fetch('/login', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: new URLSearchParams({
        email: 'admin@example.com',
        password: 'password'
    })
});

const data = await response.json();
if (data.success) {
    localStorage.setItem('auth_token', data.token);
    window.location.href = '/dashboard';
}
```

### API Calls with Authentication

```javascript
// Get current user info
const userResponse = await fetch('/api/v1/auth/user', {
    headers: {
        'Authorization': 'Bearer ' + localStorage.getItem('auth_token')
    }
});

// Create new user (Admin/System only)
const createResponse = await fetch('/api/v1/admin/users', {
    method: 'POST',
    headers: {
        'Authorization': 'Bearer ' + localStorage.getItem('auth_token'),
        'Content-Type': 'application/x-www-form-urlencoded'
    },
    body: new URLSearchParams({
        name: 'New User',
        email: 'newuser@example.com',
        password: 'password123',
        role: '2', // User role
        active: '1'
    })
});
```

### Creating Controllers with Authentication

```cpp
#include "Framework/Http/Controller.h"
#include "AuthController.h"

class MyController : public Controller {
public:
    Response index(Request& request) {
        // Check authentication
        User* currentUser = AuthController::getCurrentUser(request);
        if (!currentUser) {
            JsonDocument error;
            error["success"] = false;
            error["message"] = "Authentication required";
            return Response(request.getServerRequest()).status(401).json(error);
        }
        
        JsonDocument data;
        data["message"] = "Hello, " + currentUser->getName() + "!";
        data["role"] = currentUser->getRoleString();
        
        delete currentUser;
        return Response(request.getServerRequest()).json(data);
    }
    
    Response adminOnly(Request& request) {
        User* currentUser = AuthController::getCurrentUser(request);
        if (!currentUser || !currentUser->canAccessAdmin()) {
            if (currentUser) delete currentUser;
            return Response(request.getServerRequest())
                .status(403)
                .json({{"success", false}, {"message", "Admin access required"}});
        }
        
        // Admin-only functionality here
        delete currentUser;
        return success(request.getServerRequest(), "Admin access granted");
    }
};
```

### Creating Models with Role Support

```cpp
#include "Framework/Database/Model.h"

DECLARE_MODEL(User, "users")

class User : public Model {
public:
    User() : Model("users") {}
    
    // Basic getters/setters
    String getName() const { return get("name"); }
    void setName(const String& name) { set("name", name); }
    
    String getEmail() const { return get("email"); }
    void setEmail(const String& email) { set("email", email); }
    
    // Role management
    int getRole() const { return get("role").toInt(); }
    void setRole(int role) { set("role", String(role)); }
    
    String getRoleString() const {
        int role = getRole();
        switch (role) {
            case 0: return "system";
            case 1: return "admin";
            case 2: return "user";
            default: return "unknown";
        }
    }
    
    // Permission checks
    bool isSystem() const { return getRole() == 0; }
    bool isAdmin() const { return getRole() == 1; }
    bool isUser() const { return getRole() == 2; }
    
    bool canManageUsers() const { return getRole() <= 1; } // System or Admin
    bool canAccessAdmin() const { return getRole() <= 1; }
    
    bool canEditUser(const User* targetUser) const {
        if (!targetUser) return false;
        int myRole = getRole();
        int targetRole = targetUser->getRole();
        
        // System can edit anyone
        if (myRole == 0) return true;
        // Admin can edit users but not system or other admins
        if (myRole == 1 && targetRole == 2) return true;
        // Users can only edit themselves
        if (myRole == 2 && getEmail() == targetUser->getEmail()) return true;
        
        return false;
    }
    
    // Static finders
    static User* findByEmail(const String& email);
    static std::vector<User*> byRole(int role);
    static std::vector<User*> active();
    
    bool validate() const override {
        return getName().length() > 0 && 
               getEmail().length() > 0 && 
               getEmail().indexOf('@') > 0 &&
               getRole() >= 0 && getRole() <= 2;
    }
};
```

### Defining Routes with Authentication

```cpp
void registerRoutes(Router* router) {
    AuthController* authController = new AuthController();
    UserController* userController = new UserController();
    
    // Public routes
    router->get("/login", [authController](Request& request) -> Response {
        return authController->showLogin(request);
    });
    
    router->post("/login", [authController](Request& request) -> Response {
        return authController->login(request);
    });
    
    // Protected web routes
    router->get("/dashboard", [authController](Request& request) -> Response {
        return authController->dashboard(request);
    }).name("dashboard");
    
    // API routes with middleware
    router->group("/api/v1", [&](Router& api) {
        api.middleware({"cors", "json", "ratelimit"});
        
        // Authentication endpoints
        api.group("/auth", [&](Router& auth) {
            auth.get("/user", [authController](Request& request) -> Response {
                return authController->getUserInfo(request);
            }).name("api.auth.user");
        });
        
        // Admin-only routes
        api.group("/admin", [&](Router& admin) {
            admin.middleware("auth"); // Requires authentication
            
            admin.get("/users", [userController](Request& request) -> Response {
                return userController->apiIndex(request);
            }).name("api.admin.users.index");
            
            admin.post("/users", [userController](Request& request) -> Response {
                return userController->apiStore(request);
            }).name("api.admin.users.store");
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

```json
{
  "app": {
    "name": "ESP32 MVC Framework",
    "env": "development",
    "debug": true,
    "key": "your-app-key-here"
  },
  "server": {
    "port": 80,
    "host": "0.0.0.0"
  },
  "database": {
    "default": "csv",
    "csv": {
      "path": "/database",
      "extension": ".csv"
    }
  },
  "auth": {
    "token_prefix": "demo_token_",
    "session_timeout": 3600
  },
  "features": {
    "cors": true,
    "rate_limiting": true,
    "logging": true,
    "user_registration": true,
    "file_serving": true
  }
}
```

### Security Features

- **Token-based Authentication**: Secure session management with Bearer tokens
- **Role-based Permissions**: Granular access control for different user types
- **Input Validation**: Server-side validation for all user inputs
- **CORS Protection**: Configurable cross-origin request handling
- **Rate Limiting**: Protection against API abuse
- **Secure File Serving**: Proper MIME type handling and path validation

### CSV Database Schema

Users are stored in `/data/database/users.csv` with the following structure:

```csv
id,name,email,password,active,role,created_at,updated_at
1,System User,system@example.com,system123,1,0,2025-07-03 09:00:00,2025-07-03 09:00:00
2,Admin User,admin@example.com,password,1,1,2025-07-03 10:00:00,2025-07-03 10:00:00
3,Regular User,user@example.com,123456,1,2,2025-07-03 10:01:00,2025-07-03 10:01:00
```

## Dependencies

The framework requires the following libraries (automatically managed by PlatformIO):

- `ESP32Async/ESPAsyncWebServer` - Async HTTP server
- `ESP32Async/AsyncTCP` - Async TCP library  
- `bblanchon/ArduinoJson` - JSON parsing and generation
- `intrbiz/arduino-crypto` - Cryptographic functions (for future password hashing)

## Performance & Memory

The framework is optimized for ESP32 constraints:

- **Memory Efficient**: Careful memory management with proper cleanup
- **Async Operations**: Non-blocking HTTP server for better responsiveness
- **Minimal Heap Usage**: Stack allocation where possible
- **SPIFFS Optimization**: Efficient file serving with caching
- **Real-time Monitoring**: Built-in memory and performance monitoring

### Typical Memory Usage
- **Static RAM**: ~50KB for framework core
- **Dynamic RAM**: ~30-50KB during operation (depends on concurrent requests)
- **Flash Storage**: ~200KB for framework + your application code
- **SPIFFS**: ~1MB for web assets, database, and configuration

## Development Features

- **Debug Logging**: Comprehensive serial output for debugging
- **Hot Reload**: Modify HTML/CSS/JS files and see changes immediately
- **API Testing**: Built-in endpoints for testing all functionality
- **Error Handling**: Graceful error responses with proper HTTP status codes
- **Development Tools**: Memory usage, uptime, and system statistics

## Contributing

To extend the framework:

1. **Follow MVC Patterns**: Keep controllers, models, and views separate
2. **Add Authentication**: Use `AuthController::getCurrentUser()` for protected routes
3. **Implement Permissions**: Use role-based checks for user actions
4. **Add New Middleware**: Create middleware in `Framework/Http/` for cross-cutting concerns
5. **Extend Models**: Add new CSV-based models following the User model pattern
6. **Update Documentation**: Keep README and code comments up to date

### Adding New Features

1. Create new controller in `app/Controllers/`
2. Add model if database interaction needed
3. Register routes in `app/routes.cpp`
4. Add HTML views in `data/views/`
5. Update CSS/JS in `data/assets/`
6. Test with different user roles

### Security Considerations

- Always validate user input
- Check user permissions before sensitive operations
- Use proper HTTP status codes
- Implement rate limiting for public endpoints
- Validate file uploads and serving paths
- Keep tokens secure and implement proper logout

## Troubleshooting

### Common Issues

1. **Login fails**: Check CSV database exists and demo users are populated
2. **Routes not matching**: Verify route order (exact matches before parametric)
3. **Permission denied**: Ensure user has correct role for the operation
4. **File not found**: Check SPIFFS is initialized and files are uploaded
5. **Memory issues**: Monitor heap usage and implement proper cleanup

### Debug Tips

- Enable debug mode in `config.json`
- Check serial output for detailed logs
- Use `/api/v1/status` endpoint for system information
- Verify token format and authentication headers
- Test API endpoints with tools like curl or Postman

## Source Libraries

The following libraries are used as the foundation for this framework:

- [ESP32Async/ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer)
- [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [intrbiz/Crypto](https://github.com/intrbiz/arduino-crypto)

## License

This project is open source and available under the MIT License.
