#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>
#include "LogHandler.h"
#include <map>
#include <vector>

struct ServoConfig {
    uint8_t pin;
    int minPulseWidth;
    int maxPulseWidth;
    int minAngle;
    int maxAngle;
    String name;
};

struct ServoStatus {
    uint8_t pin;
    String name;
    int currentAngle;
    bool isAttached;
    bool isEnabled;
    unsigned long lastUpdate;
};

class ServoManager;
extern ServoManager* _servoManager;

class ServoManager {
public:
    ServoManager();
    ~ServoManager();
    
    // Singleton pattern
    static ServoManager& getInstance() {
        if (!_servoManager) {
            _servoManager = new ServoManager();
        }
        return *_servoManager;
    }
    
    // Servo management
    bool addServo(const ServoConfig& config);
    bool removeServo(uint8_t pin);
    bool removeServo(const String& name);
    
    // Servo control
    bool setAngle(uint8_t pin, int angle);
    bool setAngle(const String& name, int angle);
    bool setAngleSmooth(uint8_t pin, int angle, int delayMs = 15);
    bool setAngleSmooth(const String& name, int angle, int delayMs = 15);
    
    // Servo state
    int getAngle(uint8_t pin);
    int getAngle(const String& name);
    bool isAttached(uint8_t pin);
    bool isAttached(const String& name);
    bool isEnabled(uint8_t pin);
    bool isEnabled(const String& name);
    
    // Servo enable/disable
    bool enableServo(uint8_t pin);
    bool enableServo(const String& name);
    bool disableServo(uint8_t pin);
    bool disableServo(const String& name);
    
    // Bulk operations
    void enableAllServos();
    void disableAllServos();
    void setAllAngles(int angle);
    void sweepAllServos(int startAngle, int endAngle, int delayMs = 15);
    
    // Information
    std::vector<ServoStatus> getAllServoStatus();
    ServoStatus getServoStatus(uint8_t pin);
    ServoStatus getServoStatus(const String& name);
    bool hasServo(uint8_t pin);
    bool hasServo(const String& name);
    size_t getServoCount();
    
    // Utility
    void printStatus();
    bool validateAngle(uint8_t pin, int angle);
    bool validateAngle(const String& name, int angle);
    
    // Preset positions (optional feature)
    bool savePreset(const String& presetName);
    bool loadPreset(const String& presetName);
    std::vector<String> getPresetNames();
    bool deletePreset(const String& presetName);
    
private:
    std::map<uint8_t, Servo*> servos;
    std::map<uint8_t, ServoConfig> configs;
    std::map<String, uint8_t> nameToPin;
    std::map<String, std::map<String, int>> presets; // presetName -> {servoName -> angle}
    
    LogHandler* log;
    
    // Helper methods
    Servo* getServoByPin(uint8_t pin);
    Servo* getServoByName(const String& name);
    uint8_t getPinByName(const String& name);
    bool isValidPin(uint8_t pin);
    void logServoAction(const String& action, uint8_t pin, const String& name = "", int angle = -1);
};
