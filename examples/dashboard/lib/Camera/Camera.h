#pragma once

#include <Arduino.h>
#include <esp_camera.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "LogHandler.h" 

struct CameraFrame {
    uint8_t* buf;
    size_t len;
    bool isAllocated;  // true if we allocated memory that needs to be freed
};

class Camera;
extern Camera* _camera;

class Camera {
public:
    Camera();
    bool init();
    CameraFrame captureJPEG();  // Always returns JPEG data
    camera_fb_t* capture();     // Original method for backward compatibility
    int getQuality() { return _quality; }
    void release(camera_fb_t* fb);
    void releaseFrame(CameraFrame& frame);
    esp_err_t setFrameSize(framesize_t size);
    esp_err_t setQuality(int quality);
    esp_err_t setBrightness(int brightness);
    esp_err_t setContrast(int contrast);
    esp_err_t setSaturation(int saturation);
    
    static Camera& getInstance() {
        if (!_camera) {
            _camera = new Camera();
            _camera->init();
        }
        return *_camera;
    }
    
private:
    bool _initialized;
    int _quality;
    LogHandler* _log;
};