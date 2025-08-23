#include "ServoConfig.h"

namespace ServoConfigs {
    namespace Generic {
        ServoConfig createServoConfig(uint8_t pin, const String& name, 
                                    int minAngle, int maxAngle,
                                    int minPulse, int maxPulse) {
            return ServoConfig{
                .pin = pin,
                .minPulseWidth = minPulse,
                .maxPulseWidth = maxPulse,
                .minAngle = minAngle,
                .maxAngle = maxAngle,
                .name = name
            };
        }
        
        ServoConfig standardServo(uint8_t pin, const String& name) {
            return createServoConfig(pin, name, 0, 180, STANDARD_MIN_PULSE, STANDARD_MAX_PULSE);
        }
        
        ServoConfig precisionServo(uint8_t pin, const String& name) {
            return createServoConfig(pin, name, 0, 180, PRECISION_MIN_PULSE, PRECISION_MAX_PULSE);
        }
        
        ServoConfig continuousServo(uint8_t pin, const String& name) {
            return createServoConfig(pin, name, 0, 270, STANDARD_MIN_PULSE, STANDARD_MAX_PULSE);
        }
    }
}

namespace ServoUtils {
    bool setupPanTiltSystem(ServoManager& manager, uint8_t panPin, uint8_t tiltPin) {
        ServoConfig panConfig = ServoConfigs::Generic::standardServo(panPin, "pan");
        ServoConfig tiltConfig = ServoConfigs::Generic::standardServo(tiltPin, "tilt");
        
        bool success = true;
        success &= manager.addServo(panConfig);
        success &= manager.addServo(tiltConfig);
        
        if (success) {
            // Set to center positions
            manager.setAngle("pan", 90);
            manager.setAngle("tilt", 90);
        }
        
        return success;
    }
    
    bool setupRoboticArm(ServoManager& manager, uint8_t basePin, uint8_t armPin, uint8_t gripperPin) {
        ServoConfig baseConfig = ServoConfigs::Generic::standardServo(basePin, "base");
        ServoConfig armConfig = ServoConfigs::Generic::standardServo(armPin, "arm");
        ServoConfig gripperConfig = ServoConfigs::Generic::precisionServo(gripperPin, "gripper");
        
        bool success = true;
        success &= manager.addServo(baseConfig);
        success &= manager.addServo(armConfig);
        success &= manager.addServo(gripperConfig);
        
        if (success) {
            // Set to safe starting positions
            manager.setAngle("base", 90);
            manager.setAngle("arm", 45);
            manager.setAngle("gripper", 90);
        }
        
        return success;
    }
    
    bool setupMultiServoArray(ServoManager& manager, const std::vector<uint8_t>& pins, const String& baseName) {
        bool success = true;
        
        for (size_t i = 0; i < pins.size(); i++) {
            String servoName = baseName + "_" + String(i + 1);
            ServoConfig config = ServoConfigs::Generic::standardServo(pins[i], servoName);
            success &= manager.addServo(config);
        }
        
        if (success) {
            // Set all to center position
            manager.setAllAngles(90);
        }
        
        return success;
    }
}
