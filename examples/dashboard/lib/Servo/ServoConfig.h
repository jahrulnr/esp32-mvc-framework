#pragma once

#include <Arduino.h>
#include "Servo.h"

// Common servo configurations for different ESP32 boards
namespace ServoConfigs {
    
    // Standard servo pulse width settings (microseconds)
    const int STANDARD_MIN_PULSE = 544;   // 0.544ms
    const int STANDARD_MAX_PULSE = 2400;  // 2.4ms
    
    // High precision servo settings
    const int PRECISION_MIN_PULSE = 500;  // 0.5ms
    const int PRECISION_MAX_PULSE = 2500; // 2.5ms
    
    // Standard servo angle ranges
    const int SERVO_180_MIN = 0;
    const int SERVO_180_MAX = 180;
    const int SERVO_270_MIN = 0;
    const int SERVO_270_MAX = 270;
    
    // Pin configurations for different ESP32 boards
    
    #ifdef BOARD_ESP32_CAM
    // ESP32-CAM available pins (avoiding camera pins)
    namespace ESP32CAM {
        const uint8_t SERVO_PIN_1 = 2;   // GPIO2 (built-in LED, use with caution)
        const uint8_t SERVO_PIN_2 = 14;  // GPIO14
        const uint8_t SERVO_PIN_3 = 15;  // GPIO15
        const uint8_t SERVO_PIN_4 = 13;  // GPIO13 (available)
        const uint8_t SERVO_PIN_5 = 12;  // GPIO12 (available)
        
        // Pan-Tilt configuration example
        const ServoConfig PAN_SERVO = {
            .pin = SERVO_PIN_1,
            .minPulseWidth = STANDARD_MIN_PULSE,
            .maxPulseWidth = STANDARD_MAX_PULSE,
            .minAngle = 0,
            .maxAngle = 180,
            .name = "pan"
        };
        
        const ServoConfig TILT_SERVO = {
            .pin = SERVO_PIN_2,
            .minPulseWidth = STANDARD_MIN_PULSE,
            .maxPulseWidth = STANDARD_MAX_PULSE,
            .minAngle = 0,
            .maxAngle = 180,
            .name = "tilt"
        };
    }
    #endif
    
    #ifdef BOARD_NODEMCU32S
    // NodeMCU-32S available pins
    namespace NodeMCU32S {
        const uint8_t SERVO_PIN_1 = 2;   // GPIO2
        const uint8_t SERVO_PIN_2 = 4;   // GPIO4
        const uint8_t SERVO_PIN_3 = 5;   // GPIO5
        const uint8_t SERVO_PIN_4 = 12;  // GPIO12
        const uint8_t SERVO_PIN_5 = 13;  // GPIO13
        const uint8_t SERVO_PIN_6 = 14;  // GPIO14
        const uint8_t SERVO_PIN_7 = 15;  // GPIO15
        const uint8_t SERVO_PIN_8 = 16;  // GPIO16
        const uint8_t SERVO_PIN_9 = 17;  // GPIO17
        const uint8_t SERVO_PIN_10 = 18; // GPIO18
        const uint8_t SERVO_PIN_11 = 19; // GPIO19
        const uint8_t SERVO_PIN_12 = 21; // GPIO21
        const uint8_t SERVO_PIN_13 = 22; // GPIO22
        const uint8_t SERVO_PIN_14 = 23; // GPIO23
        const uint8_t SERVO_PIN_15 = 25; // GPIO25
        const uint8_t SERVO_PIN_16 = 26; // GPIO26
        const uint8_t SERVO_PIN_17 = 27; // GPIO27
        const uint8_t SERVO_PIN_18 = 32; // GPIO32
        const uint8_t SERVO_PIN_19 = 33; // GPIO33
        
        // Example servo configurations
        const ServoConfig BASE_SERVO = {
            .pin = SERVO_PIN_1,
            .minPulseWidth = STANDARD_MIN_PULSE,
            .maxPulseWidth = STANDARD_MAX_PULSE,
            .minAngle = 0,
            .maxAngle = 180,
            .name = "base"
        };
        
        const ServoConfig ARM_SERVO = {
            .pin = SERVO_PIN_2,
            .minPulseWidth = STANDARD_MIN_PULSE,
            .maxPulseWidth = STANDARD_MAX_PULSE,
            .minAngle = 0,
            .maxAngle = 180,
            .name = "arm"
        };
        
        const ServoConfig GRIPPER_SERVO = {
            .pin = SERVO_PIN_3,
            .minPulseWidth = PRECISION_MIN_PULSE,
            .maxPulseWidth = PRECISION_MAX_PULSE,
            .minAngle = 0,
            .maxAngle = 180,
            .name = "gripper"
        };
    }
    #endif
    
    // Generic configurations for custom setups
    namespace Generic {
        ServoConfig createServoConfig(uint8_t pin, const String& name, 
                                    int minAngle = 0, int maxAngle = 180,
                                    int minPulse = STANDARD_MIN_PULSE, 
                                    int maxPulse = STANDARD_MAX_PULSE);
        
        // Predefined servo types
        ServoConfig standardServo(uint8_t pin, const String& name);
        ServoConfig precisionServo(uint8_t pin, const String& name);
        ServoConfig continuousServo(uint8_t pin, const String& name);
    }
}

// Utility functions for servo setup
namespace ServoUtils {
    // Initialize common servo setups
    bool setupPanTiltSystem(ServoManager& manager, uint8_t panPin, uint8_t tiltPin);
    bool setupRoboticArm(ServoManager& manager, uint8_t basePin, uint8_t armPin, uint8_t gripperPin);
    bool setupMultiServoArray(ServoManager& manager, const std::vector<uint8_t>& pins, const String& baseName);
}
