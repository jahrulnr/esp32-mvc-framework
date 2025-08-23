#include "ServoController.h"

Response ServoController::getServos(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    response["success"] = true;
    response["servo_count"] = manager.getServoCount();
    
    JsonArray servos = response["servos"].to<JsonArray>();
    std::vector<ServoStatus> statusList = manager.getAllServoStatus();
    
    for (const ServoStatus& status : statusList) {
        JsonObject servo = servos.add<JsonObject>();
        servo["pin"] = status.pin;
        servo["name"] = status.name;
        servo["current_angle"] = status.currentAngle;
        servo["is_attached"] = status.isAttached;
        servo["is_enabled"] = status.isEnabled;
        servo["last_update"] = status.lastUpdate;
    }
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response ServoController::addServo(Request& request) {
    JsonDocument response;
    
    if (!validateServoRequest(request, response)) {
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    uint8_t pin = request.input("pin").toInt();
    String name = request.input("name");
    int minAngle = request.input("min_angle", "0").toInt();
    int maxAngle = request.input("max_angle", "180").toInt();
    int minPulse = request.input("min_pulse", String(ServoConfigs::STANDARD_MIN_PULSE)).toInt();
    int maxPulse = request.input("max_pulse", String(ServoConfigs::STANDARD_MAX_PULSE)).toInt();
    
    ServoConfig config = {
        .pin = pin,
        .minPulseWidth = minPulse,
        .maxPulseWidth = maxPulse,
        .minAngle = minAngle,
        .maxAngle = maxAngle,
        .name = name
    };
    
    ServoManager& manager = ServoManager::getInstance();
    
    if (manager.addServo(config)) {
        response["success"] = true;
        response["message"] = "Servo added successfully";
        response["servo"] = servoConfigToJson(config, pin);
    } else {
        response["success"] = false;
        response["message"] = "Failed to add servo";
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 400)
        .json(response);
}

Response ServoController::removeServo(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String identifier = request.input("identifier");
    bool success = false;
    
    if (identifier.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing identifier (pin or name)";
    } else {
        // Try as pin number first, then as name
        if (identifier.toInt() > 0 || identifier == "0") {
            uint8_t pin = identifier.toInt();
            success = manager.removeServo(pin);
        } else {
            success = manager.removeServo(identifier);
        }
        
        response["success"] = success;
        response["message"] = success ? "Servo removed successfully" : "Failed to remove servo";
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 400)
        .json(response);
}

Response ServoController::setAngle(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String identifier = request.input("identifier");
    String angleStr = request.input("angle");
    
    if (identifier.isEmpty() || angleStr.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing identifier or angle";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    int angle = angleStr.toInt();
    bool success = false;
    
    // Try as pin number first, then as name
    if (identifier.toInt() > 0 || identifier == "0") {
        uint8_t pin = identifier.toInt();
        success = manager.setAngle(pin, angle);
    } else {
        success = manager.setAngle(identifier, angle);
    }
    
    response["success"] = success;
    response["message"] = success ? "Angle set successfully" : "Failed to set angle";
    
    if (success) {
        response["identifier"] = identifier;
        response["angle"] = angle;
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 400)
        .json(response);
}

Response ServoController::setAngleSmooth(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String identifier = request.input("identifier");
    String angleStr = request.input("angle");
    int delayMs = request.input("delay", "15").toInt();
    
    if (identifier.isEmpty() || angleStr.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing identifier or angle";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    int angle = angleStr.toInt();
    bool success = false;
    
    // Try as pin number first, then as name
    if (identifier.toInt() > 0 || identifier == "0") {
        uint8_t pin = identifier.toInt();
        success = manager.setAngleSmooth(pin, angle, delayMs);
    } else {
        success = manager.setAngleSmooth(identifier, angle, delayMs);
    }
    
    response["success"] = success;
    response["message"] = success ? "Smooth angle set successfully" : "Failed to set angle";
    
    if (success) {
        response["identifier"] = identifier;
        response["angle"] = angle;
        response["delay_ms"] = delayMs;
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 400)
        .json(response);
}

Response ServoController::getAngle(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String identifier = request.input("identifier");
    
    if (identifier.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing identifier";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    int angle = -1;
    
    // Try as pin number first, then as name
    if (identifier.toInt() > 0 || identifier == "0") {
        uint8_t pin = identifier.toInt();
        angle = manager.getAngle(pin);
    } else {
        angle = manager.getAngle(identifier);
    }
    
    if (angle >= 0) {
        response["success"] = true;
        response["identifier"] = identifier;
        response["angle"] = angle;
    } else {
        response["success"] = false;
        response["message"] = "Servo not found or error reading angle";
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 404)
        .json(response);
}

Response ServoController::getStatus(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String identifier = request.input("identifier");
    
    if (identifier.isEmpty()) {
        // Return status for all servos
        return getServos(request);
    }
    
    ServoStatus status;
    bool found = false;
    
    // Try as pin number first, then as name
    if (identifier.toInt() > 0 || identifier == "0") {
        uint8_t pin = identifier.toInt();
        if (manager.hasServo(pin)) {
            status = manager.getServoStatus(pin);
            found = true;
        }
    } else {
        if (manager.hasServo(identifier)) {
            status = manager.getServoStatus(identifier);
            found = true;
        }
    }
    
    if (found) {
        response["success"] = true;
        response["servo"] = servoStatusToJson(status);
    } else {
        response["success"] = false;
        response["message"] = "Servo not found";
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 404)
        .json(response);
}

Response ServoController::enableServo(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String identifier = request.input("identifier");
    
    if (identifier.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing identifier";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    bool success = false;
    
    // Try as pin number first, then as name
    if (identifier.toInt() > 0 || identifier == "0") {
        uint8_t pin = identifier.toInt();
        success = manager.enableServo(pin);
    } else {
        success = manager.enableServo(identifier);
    }
    
    response["success"] = success;
    response["message"] = success ? "Servo enabled successfully" : "Failed to enable servo";
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 400)
        .json(response);
}

Response ServoController::disableServo(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String identifier = request.input("identifier");
    
    if (identifier.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing identifier";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    bool success = false;
    
    // Try as pin number first, then as name
    if (identifier.toInt() > 0 || identifier == "0") {
        uint8_t pin = identifier.toInt();
        success = manager.disableServo(pin);
    } else {
        success = manager.disableServo(identifier);
    }
    
    response["success"] = success;
    response["message"] = success ? "Servo disabled successfully" : "Failed to disable servo";
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 400)
        .json(response);
}

Response ServoController::enableAllServos(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    manager.enableAllServos();
    
    response["success"] = true;
    response["message"] = "All servos enabled";
    response["servo_count"] = manager.getServoCount();
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response ServoController::disableAllServos(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    manager.disableAllServos();
    
    response["success"] = true;
    response["message"] = "All servos disabled";
    response["servo_count"] = manager.getServoCount();
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response ServoController::setAllAngles(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String angleStr = request.input("angle");
    
    if (angleStr.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing angle parameter";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    int angle = angleStr.toInt();
    
    if (!validateAngleRange(angle)) {
        response["success"] = false;
        response["message"] = "Invalid angle range (0-180)";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    manager.setAllAngles(angle);
    
    response["success"] = true;
    response["message"] = "All servos set to angle " + String(angle);
    response["angle"] = angle;
    response["servo_count"] = manager.getServoCount();
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response ServoController::sweepAllServos(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String startAngleStr = request.input("start_angle");
    String endAngleStr = request.input("end_angle");
    int delayMs = request.input("delay", "15").toInt();
    
    if (startAngleStr.isEmpty() || endAngleStr.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing start_angle or end_angle";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    int startAngle = startAngleStr.toInt();
    int endAngle = endAngleStr.toInt();
    
    if (!validateAngleRange(startAngle) || !validateAngleRange(endAngle)) {
        response["success"] = false;
        response["message"] = "Invalid angle range (0-180)";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    // Note: This is a blocking operation
    manager.sweepAllServos(startAngle, endAngle, delayMs);
    
    response["success"] = true;
    response["message"] = "Servo sweep completed";
    response["start_angle"] = startAngle;
    response["end_angle"] = endAngle;
    response["delay_ms"] = delayMs;
    response["servo_count"] = manager.getServoCount();
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response ServoController::savePreset(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String presetName = request.input("name");
    
    if (presetName.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing preset name";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    bool success = manager.savePreset(presetName);
    
    response["success"] = success;
    response["message"] = success ? 
        "Preset '" + presetName + "' saved successfully" : 
        "Failed to save preset '" + presetName + "'";
    response["preset_name"] = presetName;
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 400)
        .json(response);
}

Response ServoController::loadPreset(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String presetName = request.input("name");
    
    if (presetName.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing preset name";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    bool success = manager.loadPreset(presetName);
    
    response["success"] = success;
    response["message"] = success ? 
        "Preset '" + presetName + "' loaded successfully" : 
        "Failed to load preset '" + presetName + "'";
    response["preset_name"] = presetName;
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 404)
        .json(response);
}

Response ServoController::getPresets(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    std::vector<String> presetNames = manager.getPresetNames();
    
    response["success"] = true;
    response["preset_count"] = presetNames.size();
    
    JsonArray presets = response["presets"].to<JsonArray>();
    for (const String& name : presetNames) {
        presets.add(name);
    }
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response ServoController::deletePreset(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String presetName = request.input("name");
    
    if (presetName.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing preset name";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    bool success = manager.deletePreset(presetName);
    
    response["success"] = success;
    response["message"] = success ? 
        "Preset '" + presetName + "' deleted successfully" : 
        "Preset '" + presetName + "' not found";
    response["preset_name"] = presetName;
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 404)
        .json(response);
}

Response ServoController::setupPanTilt(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String panPinStr = request.input("pan_pin");
    String tiltPinStr = request.input("tilt_pin");
    
    if (panPinStr.isEmpty() || tiltPinStr.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing pan_pin or tilt_pin";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    uint8_t panPin = panPinStr.toInt();
    uint8_t tiltPin = tiltPinStr.toInt();
    
    bool success = ServoUtils::setupPanTiltSystem(manager, panPin, tiltPin);
    
    response["success"] = success;
    response["message"] = success ? 
        "Pan-tilt system setup successfully" : 
        "Failed to setup pan-tilt system";
    
    if (success) {
        response["pan_pin"] = panPin;
        response["tilt_pin"] = tiltPin;
        response["pan_angle"] = 90;
        response["tilt_angle"] = 90;
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 400)
        .json(response);
}

Response ServoController::setupRoboticArm(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String basePinStr = request.input("base_pin");
    String armPinStr = request.input("arm_pin");
    String gripperPinStr = request.input("gripper_pin");
    
    if (basePinStr.isEmpty() || armPinStr.isEmpty() || gripperPinStr.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing base_pin, arm_pin, or gripper_pin";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    uint8_t basePin = basePinStr.toInt();
    uint8_t armPin = armPinStr.toInt();
    uint8_t gripperPin = gripperPinStr.toInt();
    
    bool success = ServoUtils::setupRoboticArm(manager, basePin, armPin, gripperPin);
    
    response["success"] = success;
    response["message"] = success ? 
        "Robotic arm setup successfully" : 
        "Failed to setup robotic arm";
    
    if (success) {
        response["base_pin"] = basePin;
        response["arm_pin"] = armPin;
        response["gripper_pin"] = gripperPin;
        response["base_angle"] = 90;
        response["arm_angle"] = 45;
        response["gripper_angle"] = 90;
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 400)
        .json(response);
}

Response ServoController::setupMultiArray(Request& request) {
    JsonDocument response;
    ServoManager& manager = ServoManager::getInstance();
    
    String pinsStr = request.input("pins");
    String baseName = request.input("base_name", "servo");
    
    if (pinsStr.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing pins parameter (comma-separated list)";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    // Parse comma-separated pin list
    std::vector<uint8_t> pins;
    int start = 0;
    int commaIndex = pinsStr.indexOf(',');
    
    while (commaIndex >= 0) {
        String pinStr = pinsStr.substring(start, commaIndex);
        pinStr.trim();
        if (!pinStr.isEmpty()) {
            pins.push_back(pinStr.toInt());
        }
        start = commaIndex + 1;
        commaIndex = pinsStr.indexOf(',', start);
    }
    
    // Add the last pin
    String lastPin = pinsStr.substring(start);
    lastPin.trim();
    if (!lastPin.isEmpty()) {
        pins.push_back(lastPin.toInt());
    }
    
    if (pins.empty()) {
        response["success"] = false;
        response["message"] = "No valid pins provided";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    bool success = ServoUtils::setupMultiServoArray(manager, pins, baseName);
    
    response["success"] = success;
    response["message"] = success ? 
        "Multi-servo array setup successfully" : 
        "Failed to setup multi-servo array";
    
    if (success) {
        response["servo_count"] = pins.size();
        response["base_name"] = baseName;
        response["initial_angle"] = 90;
        
        JsonArray pinArray = response["pins"].to<JsonArray>();
        for (uint8_t pin : pins) {
            pinArray.add(pin);
        }
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 400)
        .json(response);
}

// Private helper methods
JsonDocument ServoController::servoStatusToJson(const ServoStatus& status) {
    JsonDocument doc;
    doc["pin"] = status.pin;
    doc["name"] = status.name;
    doc["current_angle"] = status.currentAngle;
    doc["is_attached"] = status.isAttached;
    doc["is_enabled"] = status.isEnabled;
    doc["last_update"] = status.lastUpdate;
    return doc;
}

JsonDocument ServoController::servoConfigToJson(const ServoConfig& config, uint8_t pin) {
    JsonDocument doc;
    doc["pin"] = pin;
    doc["name"] = config.name;
    doc["min_angle"] = config.minAngle;
    doc["max_angle"] = config.maxAngle;
    doc["min_pulse_width"] = config.minPulseWidth;
    doc["max_pulse_width"] = config.maxPulseWidth;
    return doc;
}

bool ServoController::validateServoRequest(Request& request, JsonDocument& response) {
    String pinStr = request.input("pin");
    String name = request.input("name");
    
    if (pinStr.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing pin parameter";
        return false;
    }
    
    if (name.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing name parameter";
        return false;
    }
    
    uint8_t pin = pinStr.toInt();
    if (pin == 0 && pinStr != "0") {
        response["success"] = false;
        response["message"] = "Invalid pin number";
        return false;
    }
    
    return true;
}

bool ServoController::validateAngleRange(int angle, int minAngle, int maxAngle) {
    return angle >= minAngle && angle <= maxAngle;
}

// Database management methods

Response ServoController::saveConfigToDatabase(Request& request) {
    JsonDocument response;
    
    String identifier = request.input("identifier");
    
    if (identifier.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing servo identifier (pin or name)";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    ServoManager& manager = ServoManager::getInstance();
    ServoStatus status;
    bool found = false;
    
    // Try to find servo by pin or name
    if (identifier.toInt() > 0 || identifier == "0") {
        uint8_t pin = identifier.toInt();
        if (manager.hasServo(pin)) {
            status = manager.getServoStatus(pin);
            found = true;
        }
    } else {
        if (manager.hasServo(identifier)) {
            status = manager.getServoStatus(identifier);
            found = true;
        }
    }
    
    if (!found) {
        response["success"] = false;
        response["message"] = "Servo not found: " + identifier;
        return Response(request.getServerRequest())
            .status(404)
            .json(response);
    }
    
    // Create a basic config (we'd need to enhance ServoManager to expose full config)
    ServoConfig config;
    config.pin = status.pin;
    config.name = status.name;
    config.minPulseWidth = 500;  // Default values - should be actual values from manager
    config.maxPulseWidth = 2500;
    config.minAngle = 0;
    config.maxAngle = 180;
    
    bool success = ServoConfigModel::saveConfig(config);
    
    response["success"] = success;
    response["message"] = success ? 
        "Servo configuration saved to database" : 
        "Failed to save servo configuration";
    
    if (success) {
        response["config"] = servoConfigToJson(config, config.pin);
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 500)
        .json(response);
}

Response ServoController::loadConfigFromDatabase(Request& request) {
    JsonDocument response;
    
    String identifier = request.input("identifier");
    
    if (identifier.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing servo identifier (pin or name)";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    ServoConfigModel* configModel = nullptr;
    
    // Try to find by pin first, then by name
    if (identifier.toInt() > 0 || identifier == "0") {
        uint8_t pin = identifier.toInt();
        configModel = ServoConfigModel::findByPin(pin);
    } else {
        configModel = ServoConfigModel::findByName(identifier);
    }
    
    if (!configModel) {
        response["success"] = false;
        response["message"] = "Servo configuration not found in database: " + identifier;
        return Response(request.getServerRequest())
            .status(404)
            .json(response);
    }
    
    ServoConfig config = configModel->toServoConfig();
    ServoManager& manager = ServoManager::getInstance();
    
    bool success = manager.addServo(config);
    
    response["success"] = success;
    response["message"] = success ? 
        "Servo configuration loaded from database" : 
        "Failed to load servo configuration";
    
    if (success) {
        response["config"] = servoConfigToJson(config, config.pin);
    }
    
    delete configModel;
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 500)
        .json(response);
}

Response ServoController::getSavedConfigs(Request& request) {
    JsonDocument response;
    
    std::vector<ServoConfigModel*> configs = ServoConfigModel::getAll();
    
    response["success"] = true;
    response["config_count"] = configs.size();
    
    JsonArray configArray = response["configs"].to<JsonArray>();
    for (ServoConfigModel* configModel : configs) {
        JsonDocument configJson = configModel->toJson();
        configArray.add(configJson);
        delete configModel;
    }
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response ServoController::deleteSavedConfig(Request& request) {
    JsonDocument response;
    
    String identifier = request.input("identifier");
    
    if (identifier.isEmpty()) {
        response["success"] = false;
        response["message"] = "Missing servo identifier (pin or name)";
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    bool success = false;
    
    // Try to delete by pin first, then by name
    if (identifier.toInt() > 0 || identifier == "0") {
        uint8_t pin = identifier.toInt();
        success = ServoConfigModel::deleteByPin(pin);
    } else {
        success = ServoConfigModel::deleteByName(identifier);
    }
    
    response["success"] = success;
    response["message"] = success ? 
        "Servo configuration deleted from database" : 
        "Servo configuration not found in database";
    response["identifier"] = identifier;
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 404)
        .json(response);
}

Response ServoController::saveAllConfigs(Request& request) {
    JsonDocument response;
    
    ServoManager& manager = ServoManager::getInstance();
    std::vector<ServoStatus> statusList = manager.getAllServoStatus();
    
    int savedCount = 0;
    int totalCount = statusList.size();
    
    for (const ServoStatus& status : statusList) {
        // Create a basic config (enhancement needed for full config data)
        ServoConfig config;
        config.pin = status.pin;
        config.name = status.name;
        config.minPulseWidth = 500;  // Default values
        config.maxPulseWidth = 2500;
        config.minAngle = 0;
        config.maxAngle = 180;
        
        if (ServoConfigModel::saveConfig(config)) {
            savedCount++;
        }
    }
    
    bool allSaved = (savedCount == totalCount);
    
    response["success"] = allSaved;
    response["message"] = allSaved ? 
        "All servo configurations saved to database" : 
        "Some configurations failed to save";
    response["total_servos"] = totalCount;
    response["saved_count"] = savedCount;
    response["failed_count"] = totalCount - savedCount;
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 207) // 207 = Multi-Status for partial success
        .json(response);
}

Response ServoController::loadAllConfigs(Request& request) {
    JsonDocument response;
    
    std::vector<ServoConfigModel*> configs = ServoConfigModel::getAll();
    ServoManager& manager = ServoManager::getInstance();
    
    int loadedCount = 0;
    int totalCount = configs.size();
    
    for (ServoConfigModel* configModel : configs) {
        ServoConfig config = configModel->toServoConfig();
        
        if (manager.addServo(config)) {
            loadedCount++;
        }
        
        delete configModel;
    }
    
    bool allLoaded = (loadedCount == totalCount);
    
    response["success"] = allLoaded;
    response["message"] = allLoaded ? 
        "All servo configurations loaded from database" : 
        "Some configurations failed to load";
    response["total_configs"] = totalCount;
    response["loaded_count"] = loadedCount;
    response["failed_count"] = totalCount - loadedCount;
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 207) // 207 = Multi-Status for partial success
        .json(response);
}
