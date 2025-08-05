# ESP32 MVC Framework Examples

This directory contains example implementations showing how to extend the ESP32 MVC Framework.

## Examples Overview

### 1. SensorController
An example controller that demonstrates:
- Reading sensor data
- Returning JSON responses
- Using middleware
- Parameter validation

### 2. DeviceModel
An example model that shows:
- Custom attributes and methods
- Data validation
- Mock database operations
- Serialization

### 3. CustomMiddleware
Examples of custom middleware:
- API key authentication
- Request logging with custom format
- Response time tracking

## How to Use Examples

1. Copy the example files to your main `Controllers/` or `Models/` directories
2. Include them in your route definitions
3. Customize as needed for your application

## File Structure

```
examples/
├── Controllers/
│   └── SensorController.h/.cpp
├── Models/
│   └── Device.h/.cpp
├── Middleware/
│   └── CustomMiddleware.h/.cpp
└── routes_examples.cpp
```
