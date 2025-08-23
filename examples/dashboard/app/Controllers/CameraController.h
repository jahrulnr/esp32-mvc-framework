#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "Http/Controller.h"
#include "Http/Request.h"
#include "Http/Response.h"
#include "Camera.h"
#include <ArduinoJson.h>

class CameraController : public Controller {
public:
    // Camera settings endpoints
    static Response getSettings(Request& request);
    static Response updateSettings(Request& request);
    
    // Camera capture endpoints
    static Response capture(Request& request);
    
    // Camera status
    static Response getStatus(Request& request);
    
    // Camera control
    static Response enable(Request& request);
    static Response disable(Request& request);

private:
    static bool cameraEnabled;
    static void loadDefaultSettings();
    static JsonDocument getCurrentSettings();
    static bool applyCameraSettings(const JsonDocument& settings);
};

#endif
