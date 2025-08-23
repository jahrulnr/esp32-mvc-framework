# ESP32 MVC Framework - Interface API (IFA)

This document describes the available HTTP and WebSocket APIs exposed by the ESP32 application, including endpoints, expected request payloads, and example responses.

---
## Base URL
```
http://<device-ip>/api/v1
```
All endpoints below are relative to the base unless otherwise specified.

Authentication (where required) uses a simple bearer token returned by the /auth/login endpoint. Include header:
```
Authorization: Bearer <token>
Content-Type: application/x-www-form-urlencoded OR application/json (where noted)
```

---
## 1. Authentication
### POST /login (web root, not versioned) -> /login
Form fields:
- username (string, required)
- password (string, required)
- redirect (string, optional, default: /dashboard)

Response 200 / 401 / 400:
```
{
  "success": true,
  "message": "Login successful",
  "token": "cozmo_token_admin_12345678",
  "redirect": "/dashboard",
  "user": { "username": "admin" }
}
```
Error (401):
```
{ "success": false, "message": "Invalid username or password" }
```

### POST /logout
Response:
```
{ "success": true, "message": "Logged out successfully", "redirect": "/login" }
```

### GET /api/v1/auth/user
Headers: Authorization: Bearer <token>
Response 200:
```
{
  "success": true,
  "user": {
    "username": "admin",
    "permissions": {
      "canManageUsers": true,
      "canRestartSystem": true
    },
    "role": "admin"
  }
}
```
Error 401:
```
{ "success": false, "message": "Authentication required or user not found" }
```

### POST /api/v1/auth/password (Not Implemented)
Always returns:
```
{ "success": false, "message": "Password update not implemented yet" }
```

---
## 2. Admin
(All require Authorization header and admin user.)

### GET /api/v1/admin/users
Demo response:
```
{
  "success": true,
  "users": [
    {"id":1,"name":"Admin User","username":"admin","role":"admin","active":true},
    {"id":2,"name":"Regular User","username":"user","role":"user","active":true}
  ]
}
```

---
## 3. Camera
(All camera endpoints require Authorization + admin unless middleware changes.)
Base: /api/v1/camera

### GET /settings
Response 200:
```
{
  "success": true,
  "settings": {
    "quality": <int>,
    "brightness": 0,
    "contrast": 0,
    "framesize": 5
  }
}
```

### POST /settings
Form fields (integers required): quality, brightness, contrast, framesize
Success 200:
```
{
  "success": true,
  "message": "Camera settings updated successfully",
  "settings": { "quality": 10, "brightness": 0, "contrast": 0, "framesize": 5 }
}
```
Validation error 400:
```
{
  "success": false,
  "message": "Invalid Data format: All fields must be integers",
  "data": { "quality":"x","brightness":"","contrast":"","framesize":"" }
}
```

### POST /capture
Returns raw JPEG binary (Content-Type: image/jpeg) on success or JSON error:
```
{ "success": false, "message": "Failed to capture image" }
```
If camera disabled:
```
{ "success": false, "message": "Camera is disabled" }
```

### GET /status
```
{
  "success": true,
  "enabled": true,
  "initialized": true,
  "settings": { "quality": 10, "brightness": 0, "contrast": 0, "framesize": 5 }
}
```

### POST /enable
```
{ "success": true, "message": "Camera enabled", "enabled": true }
```

### POST /disable
```
{ "success": true, "message": "Camera disabled", "enabled": false }
```

---
## 4. System
(All require Authorization + admin.)
Base: /api/v1/system

### GET /stats
```
{
  "success": true,
  "data": {
    "uptime": 123456,
    "uptime_formatted": "0:02:03",
    "memory": { ... },
    "network": { ... },
    "hardware": { ... },
    "software": { ... }
  }
}
```

### GET /memory
```
{
  "success": true,
  "memory": {
    "free_heap": 123456,
    "total_heap": 200000,
    "used_heap": 76544,
    "largest_free_block": 54321,
    "heap_fragmentation": 12,
    "psram_found": false,
    "flash_size": 4194304,
    "flash_speed": 80000000
  }
}
```

### GET /network
```
{
  "success": true,
  "network": {
    "connected": true,
    "ip": "192.168.1.25",
    "gateway": "192.168.1.1",
    "subnet": "255.255.255.0",
    "dns": "192.168.1.1",
    "ssid": "MyWiFi",
    "rssi": -62,
    "mac": "AA:BB:CC:DD:EE:FF"
  }
}
```
Disconnected:
```
{ "success": true, "network": { "connected": false, "status": "Disconnected" } }
```

### GET /hostname
```
{
  "success": true,
  "hostname": "pio-esp32-cam",
  "current": "pio-esp32-cam",
  "mdns": "pio-esp32-cam.local"
}
```

### POST /hostname
Form fields:
- hostname (string, required, 1-32 chars, letters/numbers/hyphens only)

Success:
```
{
  "success": true,
  "message": "Hostname updated to: my-esp32-device",
  "hostname": "my-esp32-device",
  "mdns": "my-esp32-device.local",
  "restart_required": true
}
```
Error:
```
{
  "success": false,
  "message": "Hostname must contain only letters, numbers, and hyphens"
}
```

### GET /configurations
Get all system configurations stored in the database:
```
{
  "success": true,
  "configurations": [
    {"key": "hostname", "value": "pio-esp32-cam"},
    {"key": "camera_quality", "value": "10"},
    {"key": "camera_framesize", "value": "5"}
  ]
}
```

### POST /configuration
Form fields:
- key (string, required)
- value (string, required)

Success:
```
{
  "success": true,
  "message": "Configuration updated successfully",
  "key": "camera_quality",
  "value": "15",
  "restart_required": false
}
```
Error:
```
{
  "success": false,
  "message": "Failed to update configuration"
}
```

### POST /restart
```
{ "success": true, "message": "System restart initiated" }
```
Note: Device restarts ~100ms after response.

### POST /api/v1/restart (Legacy)
Same as /system/restart.

---
## 5. Servo Management
(All servo endpoints require Authorization + admin.)
Base: /api/v1/servo

**Important Note on Servo Identifiers:**
- Servo identifiers can be either pin numbers or servo names
- **Pin numbers**: Must be provided as strings that parse to numbers (e.g., "12", "0")
- **Servo names**: Any non-numeric string (e.g., "pan", "tilt", "gripper")
- **Logic**: If identifier.toInt() > 0 OR identifier == "0", it's treated as a pin number, otherwise as a name
- **Recommendation**: Use descriptive names when adding servos for easier management

### GET /list
Get all registered servos and their status:
```
{
  "success": true,
  "servo_count": 2,
  "servos": [
    {
      "pin": 14,
      "name": "pan",
      "current_angle": 90,
      "is_attached": true,
      "is_enabled": true,
      "last_update": 1234567890
    },
    {
      "pin": 15,
      "name": "tilt", 
      "current_angle": 45,
      "is_attached": true,
      "is_enabled": true,
      "last_update": 1234567891
    }
  ]
}
```

### POST /add
Add a new servo to the system.
Form fields:
- pin (integer, required) - GPIO pin number
- name (string, required) - Unique servo name
- min_angle (integer, optional, default: 0) - Minimum angle
- max_angle (integer, optional, default: 180) - Maximum angle
- min_pulse (integer, optional, default: 544) - Minimum pulse width in microseconds
- max_pulse (integer, optional, default: 2400) - Maximum pulse width in microseconds

Success 200:
```
{
  "success": true,
  "message": "Servo added successfully",
  "servo": {
    "pin": 14,
    "name": "pan_servo",
    "min_angle": 0,
    "max_angle": 180,
    "min_pulse_width": 544,
    "max_pulse_width": 2400
  }
}
```
Error 400:
```
{
  "success": false,
  "message": "Failed to add servo"
}
```

### DELETE /remove
Remove a servo from the system.
Form fields:
- identifier (string, required) - Pin number or servo name

Success 200:
```
{
  "success": true,
  "message": "Servo removed successfully"
}
```

### POST /angle
Set servo to specific angle.
Form fields:
- identifier (string, required) - Pin number or servo name
- angle (integer, required) - Target angle

Success 200:
```
{
  "success": true,
  "message": "Angle set successfully",
  "identifier": "pan",
  "angle": 45
}
```
Error 400:
```
{
  "success": false,
  "message": "Failed to set angle"
}
```

### POST /angle/smooth
Set servo to specific angle with smooth movement.
Form fields:
- identifier (string, required) - Pin number or servo name
- angle (integer, required) - Target angle
- delay (integer, optional, default: 15) - Delay between steps in milliseconds

Success 200:
```
{
  "success": true,
  "message": "Smooth angle set successfully",
  "identifier": "pan",
  "angle": 135,
  "delay_ms": 20
}
```

### GET /angle
Get current angle of a servo.
Form fields:
- identifier (string, required) - Pin number or servo name

Success 200:
```
{
  "success": true,
  "identifier": "pan",
  "angle": 90
}
```
Error 404:
```
{
  "success": false,
  "message": "Servo not found or error reading angle"
}
```

### GET /status
Get status of specific servo or all servos.
Form fields:
- identifier (string, optional) - Pin number or servo name. If omitted, returns all servos

Success 200 (single servo):
```
{
  "success": true,
  "servo": {
    "pin": 14,
    "name": "pan",
    "current_angle": 90,
    "is_attached": true,
    "is_enabled": true,
    "last_update": 1234567890
  }
}
```

### POST /enable
Enable a specific servo.
Form fields:
- identifier (string, required) - Pin number or servo name

Success 200:
```
{
  "success": true,
  "message": "Servo enabled successfully"
}
```

### POST /disable
Disable a specific servo.
Form fields:
- identifier (string, required) - Pin number or servo name

Success 200:
```
{
  "success": true,
  "message": "Servo disabled successfully"
}
```

### POST /enable-all
Enable all registered servos.

Success 200:
```
{
  "success": true,
  "message": "All servos enabled",
  "servo_count": 3
}
```

### POST /disable-all
Disable all registered servos.

Success 200:
```
{
  "success": true,
  "message": "All servos disabled",
  "servo_count": 3
}
```

### POST /set-all
Set all servos to the same angle.
Form fields:
- angle (integer, required) - Target angle for all servos

Success 200:
```
{
  "success": true,
  "message": "All servos set to angle 90",
  "angle": 90,
  "servo_count": 3
}
```

### POST /sweep
Sweep all servos from start angle to end angle.
Form fields:
- start_angle (integer, required) - Starting angle
- end_angle (integer, required) - Ending angle
- delay (integer, optional, default: 15) - Delay between steps in milliseconds

Success 200:
```
{
  "success": true,
  "message": "Servo sweep completed",
  "start_angle": 0,
  "end_angle": 180,
  "delay_ms": 15,
  "servo_count": 3
}
```

### POST /preset/save
Save current servo positions as a preset.
Form fields:
- name (string, required) - Preset name

Success 200:
```
{
  "success": true,
  "message": "Preset 'home_position' saved successfully",
  "preset_name": "home_position"
}
```

### POST /preset/load
Load a saved preset (set all servos to saved positions).
Form fields:
- name (string, required) - Preset name

Success 200:
```
{
  "success": true,
  "message": "Preset 'home_position' loaded successfully",
  "preset_name": "home_position"
}
```
Error 404:
```
{
  "success": false,
  "message": "Failed to load preset 'unknown_preset'"
}
```

### GET /preset/list
Get all available preset names.

Success 200:
```
{
  "success": true,
  "preset_count": 2,
  "presets": ["home_position", "extended_position"]
}
```

### DELETE /preset/delete
Delete a saved preset.
Form fields:
- name (string, required) - Preset name

Success 200:
```
{
  "success": true,
  "message": "Preset 'old_preset' deleted successfully",
  "preset_name": "old_preset"
}
```

### POST /setup/pantilt
Quick setup for pan-tilt camera system.
Form fields:
- pan_pin (integer, required) - GPIO pin for pan servo
- tilt_pin (integer, required) - GPIO pin for tilt servo

Success 200:
```
{
  "success": true,
  "message": "Pan-tilt system setup successfully",
  "pan_pin": 14,
  "tilt_pin": 15,
  "pan_angle": 90,
  "tilt_angle": 90
}
```

### POST /setup/arm
Quick setup for 3-DOF robotic arm.
Form fields:
- base_pin (integer, required) - GPIO pin for base servo
- arm_pin (integer, required) - GPIO pin for arm servo
- gripper_pin (integer, required) - GPIO pin for gripper servo

Success 200:
```
{
  "success": true,
  "message": "Robotic arm setup successfully",
  "base_pin": 12,
  "arm_pin": 13,
  "gripper_pin": 14,
  "base_angle": 90,
  "arm_angle": 45,
  "gripper_angle": 90
}
```

### POST /setup/array
Quick setup for multiple servos with sequential naming.
Form fields:
- pins (string, required) - Comma-separated list of GPIO pins (e.g., "2,4,5,12")
- base_name (string, optional, default: "servo") - Base name for servo naming

Success 200:
```
{
  "success": true,
  "message": "Multi-servo array setup successfully",
  "servo_count": 4,
  "base_name": "joint",
  "initial_angle": 90,
  "pins": [2, 4, 5, 12]
}
```

### Database Configuration Management

#### POST /config/save
Save a single servo configuration to the database.
Form fields:
- identifier (string, required) - Pin number or servo name

Success 200:
```
{
  "success": true,
  "message": "Servo configuration saved to database",
  "config": {
    "pin": 14,
    "name": "pan_servo",
    "min_angle": 0,
    "max_angle": 180,
    "min_pulse_width": 500,
    "max_pulse_width": 2500
  }
}
```
Error 404:
```
{
  "success": false,
  "message": "Servo not found: pan"
}
```

#### POST /config/load
Load a servo configuration from the database and add it to the active servo manager.
Form fields:
- identifier (string, required) - Pin number or servo name

Success 200:
```
{
  "success": true,
  "message": "Servo configuration loaded from database",
  "config": {
    "pin": 14,
    "name": "pan_servo",
    "min_angle": 0,
    "max_angle": 180,
    "min_pulse_width": 500,
    "max_pulse_width": 2500
  }
}
```
Error 404:
```
{
  "success": false,
  "message": "Servo configuration not found in database: 14"
}
```

#### GET /config/list
Get all saved servo configurations from the database.

Success 200:
```
{
  "success": true,
  "config_count": 2,
  "configs": [
    {
      "pin": 2,
      "name": "pan_servo",
      "min_pulse_width": 500,
      "max_pulse_width": 2500,
      "min_angle": 0,
      "max_angle": 180
    },
    {
      "pin": 4,
      "name": "tilt_servo",
      "min_pulse_width": 500,
      "max_pulse_width": 2500,
      "min_angle": 0,
      "max_angle": 180
    }
  ]
}
```

#### DELETE /config/delete
Delete a servo configuration from the database.
Form fields:
- identifier (string, required) - Pin number or servo name

Success 200:
```
{
  "success": true,
  "message": "Servo configuration deleted from database",
  "identifier": "pan_servo"
}
```
Error 404:
```
{
  "success": false,
  "message": "Servo configuration not found in database"
}
```

#### POST /config/save-all
Save all currently active servo configurations to the database.

Success 200:
```
{
  "success": true,
  "message": "All servo configurations saved to database",
  "total_servos": 3,
  "saved_count": 3,
  "failed_count": 0
}
```
Partial success 207:
```
{
  "success": false,
  "message": "Some configurations failed to save",
  "total_servos": 3,
  "saved_count": 2,
  "failed_count": 1
}
```

#### POST /config/load-all
Load all saved servo configurations from the database and add them to the active servo manager.

Success 200:
```
{
  "success": true,
  "message": "All servo configurations loaded from database",
  "total_configs": 2,
  "loaded_count": 2,
  "failed_count": 0
}
```
Partial success 207:
```
{
  "success": false,
  "message": "Some configurations failed to load",
  "total_configs": 3,
  "loaded_count": 2,
  "failed_count": 1
}
```

---
## 6. WebSocket Endpoints
### /ws/camera
Usage: Connect via ws://<device-ip>/ws/camera

On connect: server sends
```
{"type":"welcome","message":"Connected to camera stream"}
```

Client -> Server JSON commands:
- {"command":"capture"} : Server responds with binary JPEG frame (followed by optional messages) or error JSON.
- {"command":"ping"} : Server responds with {"type":"pong","timestamp": <ms>}.

Error example:
```
{"type":"error","message":"Failed to capture frame"}
```

---
## 7. Static & SPA Routes (Non-API)
- GET / -> Serves /views/app.html
- GET /dashboard -> Redirects to /#dashboard
- GET /login -> Redirects to /#login
- GET /assets/{file} -> Serves /assets/<file>
- GET /favicon.ico -> Favicon

---
## 8. Notes & Conventions
- Content-Type for form submissions: application/x-www-form-urlencoded
- JSON bodies can be supported if request.input() parser handles it (current code shows form field usage).
- Tokens are simple (cozmo_token_<username>_<timestamp>) and not secure for production.
- Framesize, brightness, contrast persistence is not implemented; defaults shown.
- Servo angles are validated against configured min/max ranges for each servo.
- **Servo identifier logic**: 
  - If `identifier.toInt() > 0` OR `identifier == "0"`, treated as pin number
  - Otherwise, treated as servo name
  - Examples: "12" → pin 12, "0" → pin 0, "pan" → servo named "pan"
- Frontend uses pin numbers as object keys, but sends appropriate identifiers to API
- **Database persistence**: Servo configurations can be saved to/loaded from CSV database
  - Database table: `servo_configs` with columns: pin, name, min_pulse_width, max_pulse_width, min_angle, max_angle
  - Demo data includes pan_servo (pin 2) and tilt_servo (pin 4)
  - Configuration persistence survives system restarts
  - Use `/config/save-all` and `/config/load-all` for bulk operations

---
- Recommendation: Always provide names when adding servos for better usability

---
## 9. Quick Auth Flow
1. POST /login with credentials
2. Store token
3. Include Authorization header for protected endpoints
4. Use /api/v1/auth/user to refresh user info

---
## 10. Error Patterns
- 400: Validation errors
- 401: Authentication failures
- 404: Resource not found (servo, preset)
- 500: Internal / camera / servo failure
- 200: Success (even for some unimplemented endpoints like password update)

---
## 6. Static & SPA Routes (Non-API)
- GET / -> Serves /views/app.html
- GET /dashboard -> Redirects to /#dashboard
- GET /login -> Redirects to /#login
- GET /assets/{file} -> Serves /assets/<file>
- GET /favicon.ico -> Favicon

---
## 7. Notes & Conventions
- Content-Type for form submissions: application/x-www-form-urlencoded
- JSON bodies can be supported if request.input() parser handles it (current code shows form field usage).
- Tokens are simple (cozmo_token_<username>_<timestamp>) and not secure for production.
- Framesize, brightness, contrast persistence is not implemented; defaults shown.
- **Servo data structure**: `/api/v1/servo/list` returns an array of servo objects with `pin`, `name`, `current_angle`, `is_attached`, `is_enabled`, and `last_update` properties.

---
## 8. Quick Auth Flow
1. POST /login with credentials
2. Store token
3. Include Authorization header for protected endpoints
4. Use /api/v1/auth/user to refresh user info

---
## 9. Error Patterns
- 400: Validation errors
- 401: Authentication failures
- 500: Internal / camera failure
- 200: Success (even for some unimplemented endpoints like password update)

End of document.
