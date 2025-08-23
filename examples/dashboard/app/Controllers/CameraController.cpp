#include "CameraController.h"

// Static member initialization
bool CameraController::cameraEnabled = true; // Enable camera by default

// Camera buffer
camera_fb_t* frame = nullptr;

Response CameraController::getSettings(Request& request) {
    JsonDocument response;
    response["success"] = true;
    response["settings"] = getCurrentSettings();
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response CameraController::updateSettings(Request& request) {
    JsonDocument response;
    
    // Parse request body
    JsonDocument settings;
		// Validate and parse input as integers
		String qualityStr = request.input("quality");
		String brightnessStr = request.input("brightness");
		String contrastStr = request.input("contrast");
		String framesizeStr = request.input("framesize");

		// Helper lambda to check if string is a valid integer
		auto isInt = [](String& str) {
			if (!str || str == "") return false;
			char* endptr = nullptr;
			strtol(str.c_str(), &endptr, 10);
			return endptr && *endptr == '\0';
		};

		if (!isInt(qualityStr) || !isInt(brightnessStr) || !isInt(contrastStr) || !isInt(framesizeStr)) {
			response["success"] = false;
			response["message"] = "Invalid Data format: All fields must be integers";
			response["data"]["quality"] = qualityStr;
			response["data"]["brightness"] = brightnessStr;
			response["data"]["contrast"] = contrastStr;
			response["data"]["framesize"] = framesizeStr;
			return Response(request.getServerRequest())
				.status(400)
				.json(response);
		}

		settings["quality"] = atoi(qualityStr.c_str());
		settings["brightness"] = atoi(brightnessStr.c_str());
		settings["contrast"] = atoi(contrastStr.c_str());
		settings["framesize"] = atoi(framesizeStr.c_str());

		if (settings["framesize"] == 0) {
        response["success"] = false;
        response["message"] = "Invalid Data format";
        
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    // Apply settings
    if (applyCameraSettings(settings)) {
        response["success"] = true;
        response["message"] = "Camera settings updated successfully";
        response["settings"] = getCurrentSettings();
    } else {
        response["success"] = false;
        response["message"] = "Failed to apply camera settings";
    }
    
    return Response(request.getServerRequest())
        .status(response["success"] ? 200 : 500)
        .json(response);
}

Response CameraController::capture(Request& request) {
    Camera& camera = Camera::getInstance();
    
    if (!cameraEnabled) {
        JsonDocument response;
        response["success"] = false;
        response["message"] = "Camera is disabled";
        
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    // Capture frame
    camera.release(frame);
    frame = camera.capture();
    
    if (frame->buf && frame->len > 0) {
        // Send binary JPEG data directly using AsyncWebServerRequest
        AsyncWebServerRequest* serverRequest = request.getServerRequest();
				Response result = Response(serverRequest)
						.status(200)
						.binary((const uint8_t*)frame->buf, frame->len, "image/jpeg");
        return result;
    } else {
        JsonDocument response;
        response["success"] = false;
        response["message"] = "Failed to capture image";
        
        return Response(request.getServerRequest())
            .status(500)
            .json(response);
    }
}

Response CameraController::getStatus(Request& request) {
    JsonDocument response;
    response["success"] = true;
    response["enabled"] = cameraEnabled;
    response["initialized"] = true; // Camera is initialized in app.ino
    response["settings"] = getCurrentSettings();
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response CameraController::enable(Request& request) {
    cameraEnabled = true;
    
    JsonDocument response;
    response["success"] = true;
    response["message"] = "Camera enabled";
    response["enabled"] = cameraEnabled;
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

Response CameraController::disable(Request& request) {
    cameraEnabled = false;
    
    JsonDocument response;
    response["success"] = true;
    response["message"] = "Camera disabled";
    response["enabled"] = cameraEnabled;
    
    return Response(request.getServerRequest())
        .status(200)
        .json(response);
}

JsonDocument CameraController::getCurrentSettings() {
    JsonDocument settings;
    Camera& camera = Camera::getInstance();
    
    settings["quality"] = camera.getQuality();
    settings["brightness"] = 0; // Default value, would need to be stored/retrieved
    settings["contrast"] = 0;   // Default value, would need to be stored/retrieved
    settings["framesize"] = 5;  // Default QVGA, would need to be stored/retrieved
    
    return settings;
}

bool CameraController::applyCameraSettings(const JsonDocument& settings) {
    Camera& camera = Camera::getInstance();
    bool success = true;
    
    try {
        if (settings["quality"].is<int>()) {
            int quality = settings["quality"];
            if (camera.setQuality(quality) != ESP_OK) {
                success = false;
            }
        }
        
        if (settings["brightness"].is<int>()) {
            int brightness = settings["brightness"];
            if (camera.setBrightness(brightness) != ESP_OK) {
                success = false;
            }
        }
        
        if (settings["contrast"].is<int>()) {
            int contrast = settings["contrast"];
            if (camera.setContrast(contrast) != ESP_OK) {
                success = false;
            }
        }
        
        if (settings["framesize"].is<int>()) {
            int framesize = settings["framesize"];
            if (camera.setFrameSize((framesize_t)framesize) != ESP_OK) {
                success = false;
            }
        }
        
    } catch (...) {
        success = false;
    }
    
    return success;
}