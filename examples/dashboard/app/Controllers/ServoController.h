#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include "Http/Controller.h"
#include "Http/Request.h"
#include "Http/Response.h"
#include "Servo.h"
#include "ServoConfig.h"
#include "../Models/ServoConfig.h"
#include <ArduinoJson.h>

class ServoController : public Controller {
public:
    // Servo management endpoints
    static Response getServos(Request& request);
    static Response addServo(Request& request);
    static Response removeServo(Request& request);
    
    // Servo control endpoints
    static Response setAngle(Request& request);
    static Response setAngleSmooth(Request& request);
    static Response getAngle(Request& request);
    
    // Servo status endpoints
    static Response getStatus(Request& request);
    static Response enableServo(Request& request);
    static Response disableServo(Request& request);
    
    // Bulk operations
    static Response enableAllServos(Request& request);
    static Response disableAllServos(Request& request);
    static Response setAllAngles(Request& request);
    static Response sweepAllServos(Request& request);
    
    // Preset management
    static Response savePreset(Request& request);
    static Response loadPreset(Request& request);
    static Response getPresets(Request& request);
    static Response deletePreset(Request& request);
    
    // Quick setup endpoints
    static Response setupPanTilt(Request& request);
    static Response setupRoboticArm(Request& request);
    static Response setupMultiArray(Request& request);
    
    // Database management endpoints
    static Response saveConfigToDatabase(Request& request);
    static Response loadConfigFromDatabase(Request& request);
    static Response getSavedConfigs(Request& request);
    static Response deleteSavedConfig(Request& request);
    static Response saveAllConfigs(Request& request);
    static Response loadAllConfigs(Request& request);

private:
    static JsonDocument servoStatusToJson(const ServoStatus& status);
    static JsonDocument servoConfigToJson(const ServoConfig& config, uint8_t pin);
    static bool validateServoRequest(Request& request, JsonDocument& response);
    static bool validateAngleRange(int angle, int minAngle = 0, int maxAngle = 180);
};

#endif
