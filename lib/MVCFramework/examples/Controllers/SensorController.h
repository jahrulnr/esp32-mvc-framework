#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include "../../app/Framework/Http/Controller.h"

class SensorController : public ResourceController {
private:
    float temperature = 25.0;
    float humidity = 60.0;
    float pressure = 1013.25;

public:
    // GET /sensors - List all sensor readings
    Response index(Request& request) override;
    
    // GET /sensors/{type} - Get specific sensor reading
    Response show(Request& request) override;
    
    // POST /sensors/calibrate - Calibrate sensors
    Response calibrate(Request& request);
    
    // GET /sensors/stats - Get sensor statistics
    Response stats(Request& request);
    
private:
    float readTemperature();
    float readHumidity();
    float readPressure();
    JsonDocument formatSensorData(const String& type, float value, const String& unit);
};

#endif
