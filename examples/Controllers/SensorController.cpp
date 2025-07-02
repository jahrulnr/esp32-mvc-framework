#include "SensorController.h"

Response SensorController::index(Request& request) {
    JsonDocument data;
    data["sensors"] = JsonArray();
    
    // Add temperature sensor
    JsonObject tempSensor = data["sensors"].add<JsonObject>();
    tempSensor["type"] = "temperature";
    tempSensor["value"] = readTemperature();
    tempSensor["unit"] = "°C";
    tempSensor["timestamp"] = millis();
    
    // Add humidity sensor
    JsonObject humSensor = data["sensors"].add<JsonObject>();
    humSensor["type"] = "humidity";
    humSensor["value"] = readHumidity();
    humSensor["unit"] = "%";
    humSensor["timestamp"] = millis();
    
    // Add pressure sensor
    JsonObject presSensor = data["sensors"].add<JsonObject>();
    presSensor["type"] = "pressure";
    presSensor["value"] = readPressure();
    presSensor["unit"] = "hPa";
    presSensor["timestamp"] = millis();
    
    data["count"] = 3;
    data["status"] = "active";
    
    return Response(request.getServerRequest()).json(data);
}

Response SensorController::show(Request& request) {
    String sensorType = request.route("type");
    
    if (sensorType.length() == 0) {
        return error(request.getServerRequest(), "Sensor type is required", 400);
    }
    
    JsonDocument data;
    
    if (sensorType == "temperature") {
        data = formatSensorData("temperature", readTemperature(), "°C");
    } else if (sensorType == "humidity") {
        data = formatSensorData("humidity", readHumidity(), "%");
    } else if (sensorType == "pressure") {
        data = formatSensorData("pressure", readPressure(), "hPa");
    } else {
        return error(request.getServerRequest(), "Unknown sensor type: " + sensorType, 404);
    }
    
    return Response(request.getServerRequest()).json(data);
}

Response SensorController::calibrate(Request& request) {
    String sensorType = request.input("type");
    String offsetStr = request.input("offset");
    
    if (sensorType.length() == 0) {
        return error(request.getServerRequest(), "Sensor type is required", 400);
    }
    
    float offset = offsetStr.length() > 0 ? offsetStr.toFloat() : 0.0;
    
    // Simulate calibration
    JsonDocument response;
    response["message"] = "Sensor " + sensorType + " calibrated successfully";
    response["type"] = sensorType;
    response["offset"] = offset;
    response["timestamp"] = millis();
    response["status"] = "calibrated";
    
    return Response(request.getServerRequest()).json(response);
}

Response SensorController::stats(Request& request) {
    JsonDocument data;
    
    // Simulate sensor statistics
    data["uptime"] = millis();
    data["readings_count"] = random(1000, 10000);
    data["last_reading"] = millis() - random(1000, 5000);
    
    data["temperature"] = JsonObject();
    data["temperature"]["min"] = 18.5;
    data["temperature"]["max"] = 32.1;
    data["temperature"]["avg"] = 25.3;
    data["temperature"]["current"] = readTemperature();
    
    data["humidity"] = JsonObject();
    data["humidity"]["min"] = 35.2;
    data["humidity"]["max"] = 85.7;
    data["humidity"]["avg"] = 62.1;
    data["humidity"]["current"] = readHumidity();
    
    data["pressure"] = JsonObject();
    data["pressure"]["min"] = 995.4;
    data["pressure"]["max"] = 1025.8;
    data["pressure"]["avg"] = 1013.2;
    data["pressure"]["current"] = readPressure();
    
    return Response(request.getServerRequest()).json(data);
}

float SensorController::readTemperature() {
    // Simulate temperature reading with some variation
    return temperature + random(-500, 500) / 100.0;
}

float SensorController::readHumidity() {
    // Simulate humidity reading with some variation
    return humidity + random(-1000, 1000) / 100.0;
}

float SensorController::readPressure() {
    // Simulate pressure reading with some variation
    return pressure + random(-200, 200) / 100.0;
}

JsonDocument SensorController::formatSensorData(const String& type, float value, const String& unit) {
    JsonDocument data;
    data["type"] = type;
    data["value"] = value;
    data["unit"] = unit;
    data["timestamp"] = millis();
    data["status"] = "active";
    
    // Add sensor-specific metadata
    if (type == "temperature") {
        data["range"] = JsonObject();
        data["range"]["min"] = -40;
        data["range"]["max"] = 125;
        data["accuracy"] = "±0.5°C";
    } else if (type == "humidity") {
        data["range"] = JsonObject();
        data["range"]["min"] = 0;
        data["range"]["max"] = 100;
        data["accuracy"] = "±3%";
    } else if (type == "pressure") {
        data["range"] = JsonObject();
        data["range"]["min"] = 300;
        data["range"]["max"] = 1100;
        data["accuracy"] = "±1 hPa";
    }
    
    return data;
}
