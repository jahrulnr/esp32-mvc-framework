#include "Servo.h"

ServoManager* _servoManager = nullptr;

ServoManager::ServoManager() {
    log = new LogHandler("ServoManager");
    log->info("ServoManager initialized");
}

ServoManager::~ServoManager() {
    // Cleanup all servos
    for (auto& pair : servos) {
        if (pair.second && pair.second->attached()) {
            pair.second->detach();
        }
        delete pair.second;
    }
    servos.clear();
    configs.clear();
    nameToPin.clear();
    presets.clear();
    
    delete log;
}

bool ServoManager::addServo(const ServoConfig& config) {
    if (!isValidPin(config.pin)) {
        log->error("Invalid pin %d for servo %s", config.pin, config.name.c_str());
        return false;
    }
    
    if (hasServo(config.pin)) {
        log->warn("Servo already exists on pin %d", config.pin);
        return false;
    }
    
    if (hasServo(config.name)) {
        log->warn("Servo with name '%s' already exists", config.name.c_str());
        return false;
    }
    
    Servo* servo = new Servo();
    
    // Set timing if provided
    if (config.minPulseWidth > 0 && config.maxPulseWidth > 0) {
        servo->setPeriodHertz(50); // Standard 50Hz servo frequency
        if (servo->attach(config.pin, config.minPulseWidth, config.maxPulseWidth) < 0) {
            log->error("Failed to attach servo on pin %d", config.pin);
            delete servo;
            return false;
        }
    } else {
        if (servo->attach(config.pin) < 0) {
            log->error("Failed to attach servo on pin %d", config.pin);
            delete servo;
            return false;
        }
    }
    
    servos[config.pin] = servo;
    configs[config.pin] = config;
    nameToPin[config.name] = config.pin;
    
    // Set to middle position initially
    int middleAngle = (config.maxAngle + config.minAngle) / 2;
    servo->write(middleAngle);
    
    logServoAction("Added", config.pin, config.name, middleAngle);
    return true;
}

bool ServoManager::removeServo(uint8_t pin) {
    if (!hasServo(pin)) {
        log->warn("No servo found on pin %d", pin);
        return false;
    }
    
    Servo* servo = servos[pin];
    String name = configs[pin].name;
    
    if (servo->attached()) {
        servo->detach();
    }
    delete servo;
    
    servos.erase(pin);
    nameToPin.erase(name);
    configs.erase(pin);
    
    logServoAction("Removed", pin, name);
    return true;
}

bool ServoManager::removeServo(const String& name) {
    if (!hasServo(name)) {
        log->warn("No servo found with name '%s'", name.c_str());
        return false;
    }
    
    uint8_t pin = getPinByName(name);
    return removeServo(pin);
}

bool ServoManager::setAngle(uint8_t pin, int angle) {
    Servo* servo = getServoByPin(pin);
    if (!servo) {
        return false;
    }
    
    if (!validateAngle(pin, angle)) {
        return false;
    }
    
    servo->write(angle);
    logServoAction("SetAngle", pin, configs[pin].name, angle);
    return true;
}

bool ServoManager::setAngle(const String& name, int angle) {
    uint8_t pin = getPinByName(name);
    if (pin == 0) {
        return false;
    }
    return setAngle(pin, angle);
}

bool ServoManager::setAngleSmooth(uint8_t pin, int angle, int delayMs) {
    Servo* servo = getServoByPin(pin);
    if (!servo) {
        return false;
    }
    
    if (!validateAngle(pin, angle)) {
        return false;
    }
    
    int currentAngle = servo->read();
    int step = (angle > currentAngle) ? 1 : -1;
    
    for (int pos = currentAngle; pos != angle; pos += step) {
        servo->write(pos);
        delay(delayMs);
    }
    servo->write(angle); // Ensure we reach the exact target
    
    logServoAction("SetAngleSmooth", pin, configs[pin].name, angle);
    return true;
}

bool ServoManager::setAngleSmooth(const String& name, int angle, int delayMs) {
    uint8_t pin = getPinByName(name);
    if (pin == 0) {
        return false;
    }
    return setAngleSmooth(pin, angle, delayMs);
}

int ServoManager::getAngle(uint8_t pin) {
    Servo* servo = getServoByPin(pin);
    if (!servo) {
        return -1;
    }
    return servo->read();
}

int ServoManager::getAngle(const String& name) {
    uint8_t pin = getPinByName(name);
    if (pin == 0) {
        return -1;
    }
    return getAngle(pin);
}

bool ServoManager::isAttached(uint8_t pin) {
    Servo* servo = getServoByPin(pin);
    return servo && servo->attached();
}

bool ServoManager::isAttached(const String& name) {
    uint8_t pin = getPinByName(name);
    if (pin == 0) {
        return false;
    }
    return isAttached(pin);
}

bool ServoManager::isEnabled(uint8_t pin) {
    return isAttached(pin); // For Servo, attached means enabled
}

bool ServoManager::isEnabled(const String& name) {
    uint8_t pin = getPinByName(name);
    if (pin == 0) {
        return false;
    }
    return isEnabled(pin);
}

bool ServoManager::enableServo(uint8_t pin) {
    if (!hasServo(pin)) {
        return false;
    }
    
    Servo* servo = servos[pin];
    if (!servo->attached()) {
        ServoConfig& config = configs[pin];
        bool success;
        if (config.minPulseWidth > 0 && config.maxPulseWidth > 0) {
            success = servo->attach(config.pin, config.minPulseWidth, config.maxPulseWidth);
        } else {
            success = servo->attach(config.pin);
        }
        
        if (success) {
            logServoAction("Enabled", pin, config.name);
        }
        return success;
    }
    return true; // Already enabled
}

bool ServoManager::enableServo(const String& name) {
    uint8_t pin = getPinByName(name);
    if (pin == 0) {
        return false;
    }
    return enableServo(pin);
}

bool ServoManager::disableServo(uint8_t pin) {
    Servo* servo = getServoByPin(pin);
    if (!servo) {
        return false;
    }
    
    if (servo->attached()) {
        servo->detach();
        logServoAction("Disabled", pin, configs[pin].name);
    }
    return true;
}

bool ServoManager::disableServo(const String& name) {
    uint8_t pin = getPinByName(name);
    if (pin == 0) {
        return false;
    }
    return disableServo(pin);
}

void ServoManager::enableAllServos() {
    for (auto& pair : configs) {
        enableServo(pair.first);
    }
    log->info("Enabled all servos");
}

void ServoManager::disableAllServos() {
    for (auto& pair : servos) {
        disableServo(pair.first);
    }
    log->info("Disabled all servos");
}

void ServoManager::setAllAngles(int angle) {
    for (auto& pair : configs) {
        setAngle(pair.first, angle);
    }
    log->info("Set all servos to angle %d", angle);
}

void ServoManager::sweepAllServos(int startAngle, int endAngle, int delayMs) {
    log->info("Sweeping all servos from %d to %d", startAngle, endAngle);
    
    int step = (endAngle > startAngle) ? 1 : -1;
    for (int angle = startAngle; angle != endAngle + step; angle += step) {
        for (auto& pair : configs) {
            if (validateAngle(pair.first, angle)) {
                Servo* servo = servos[pair.first];
                if (servo && servo->attached()) {
                    servo->write(angle);
                }
            }
        }
        delay(delayMs);
    }
}

std::vector<ServoStatus> ServoManager::getAllServoStatus() {
    std::vector<ServoStatus> statusList;
    
    for (auto& pair : configs) {
        ServoStatus status = getServoStatus(pair.first);
        statusList.push_back(status);
    }
    
    return statusList;
}

ServoStatus ServoManager::getServoStatus(uint8_t pin) {
    ServoStatus status = {};
    
    if (hasServo(pin)) {
        ServoConfig& config = configs[pin];
        Servo* servo = servos[pin];
        
        status.pin = pin;
        status.name = config.name;
        status.currentAngle = servo->read();
        status.isAttached = servo->attached();
        status.isEnabled = status.isAttached;
        status.lastUpdate = millis();
    }
    
    return status;
}

ServoStatus ServoManager::getServoStatus(const String& name) {
    uint8_t pin = getPinByName(name);
    if (pin == 0) {
        return {};
    }
    return getServoStatus(pin);
}

bool ServoManager::hasServo(uint8_t pin) {
    return servos.find(pin) != servos.end();
}

bool ServoManager::hasServo(const String& name) {
    return nameToPin.find(name) != nameToPin.end();
}

size_t ServoManager::getServoCount() {
    return servos.size();
}

void ServoManager::printStatus() {
    log->info("=== Servo Manager Status ===");
    log->info("Total servos: %zu", getServoCount());
    
    for (auto& pair : configs) {
        ServoStatus status = getServoStatus(pair.first);
        log->info("Servo '%s' (Pin %d): Angle=%d, Attached=%s, Enabled=%s", 
                 status.name.c_str(), 
                 status.pin, 
                 status.currentAngle,
                 status.isAttached ? "Yes" : "No",
                 status.isEnabled ? "Yes" : "No");
    }
}

bool ServoManager::validateAngle(uint8_t pin, int angle) {
    if (!hasServo(pin)) {
        log->error("No servo found on pin %d", pin);
        return false;
    }
    
    ServoConfig& config = configs[pin];
    if (angle < config.minAngle || angle > config.maxAngle) {
        log->error("Angle %d out of range [%d, %d] for servo '%s'", 
                   angle, config.minAngle, config.maxAngle, config.name.c_str());
        return false;
    }
    
    return true;
}

bool ServoManager::validateAngle(const String& name, int angle) {
    uint8_t pin = getPinByName(name);
    if (pin == 0) {
        return false;
    }
    return validateAngle(pin, angle);
}

bool ServoManager::savePreset(const String& presetName) {
    if (servos.empty()) {
        log->warn("No servos to save in preset '%s'", presetName.c_str());
        return false;
    }
    
    std::map<String, int> preset;
    for (auto& pair : configs) {
        Servo* servo = servos[pair.first];
        if (servo && servo->attached()) {
            preset[pair.second.name] = servo->read();
        }
    }
    
    presets[presetName] = preset;
    log->info("Saved preset '%s' with %zu servo positions", presetName.c_str(), preset.size());
    return true;
}

bool ServoManager::loadPreset(const String& presetName) {
    if (presets.find(presetName) == presets.end()) {
        log->error("Preset '%s' not found", presetName.c_str());
        return false;
    }
    
    std::map<String, int>& preset = presets[presetName];
    int successCount = 0;
    
    for (auto& pair : preset) {
        if (setAngle(pair.first, pair.second)) {
            successCount++;
        }
    }
    
    log->info("Loaded preset '%s': %d/%zu servo positions applied", 
              presetName.c_str(), successCount, preset.size());
    return successCount > 0;
}

std::vector<String> ServoManager::getPresetNames() {
    std::vector<String> names;
    for (auto& pair : presets) {
        names.push_back(pair.first);
    }
    return names;
}

bool ServoManager::deletePreset(const String& presetName) {
    if (presets.find(presetName) == presets.end()) {
        log->warn("Preset '%s' not found", presetName.c_str());
        return false;
    }
    
    presets.erase(presetName);
    log->info("Deleted preset '%s'", presetName.c_str());
    return true;
}

// Private helper methods
Servo* ServoManager::getServoByPin(uint8_t pin) {
    if (!hasServo(pin)) {
        log->error("No servo found on pin %d", pin);
        return nullptr;
    }
    return servos[pin];
}

Servo* ServoManager::getServoByName(const String& name) {
    uint8_t pin = getPinByName(name);
    if (pin == 0) {
        return nullptr;
    }
    return getServoByPin(pin);
}

uint8_t ServoManager::getPinByName(const String& name) {
    if (nameToPin.find(name) == nameToPin.end()) {
        log->error("No servo found with name '%s'", name.c_str());
        return 0;
    }
    return nameToPin[name];
}

bool ServoManager::isValidPin(uint8_t pin) {
    // ESP32 GPIO pins that support PWM (simplified list)
    static const std::vector<uint8_t> validPins = {
        2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
    };
    
    return std::find(validPins.begin(), validPins.end(), pin) != validPins.end();
}

void ServoManager::logServoAction(const String& action, uint8_t pin, const String& name, int angle) {
    if (angle >= 0) {
        log->info("%s servo '%s' on pin %d, angle: %d", 
                 action.c_str(), name.c_str(), pin, angle);
    } else {
        log->info("%s servo '%s' on pin %d", 
                 action.c_str(), name.c_str(), pin);
    }
}
