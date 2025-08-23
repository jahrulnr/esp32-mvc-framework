#include "Camera.h"
#include "CameraConfig.h"

Camera* _camera = nullptr;

Camera::Camera(): _initialized(false), _quality(FRAMESIZE_VGA) {
    _log = new LogHandler("Camera");
}

bool Camera::init() {
    #ifdef CAMERA_DISABLED
        return false;
    #endif
    if (_initialized) return true;

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.jpeg_quality = 12;  // 0-63, lower is better quality
    config.sccb_i2c_port = 0;
    config.frame_size = FRAMESIZE_VGA;
    
    // PSRAM configuration
    if (psramFound()) {
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_LATEST;
        config.fb_count = 2;
    } else {
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        config.fb_count = 1;
    }
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        _log->error("Camera init failed: %s", _log->Error(err));
        return false;
    }
    _log->info("Camera initialized successfully");
    
    return true;
}

camera_fb_t* Camera::capture() {
    #ifdef CAMERA_DISABLED
    return nullptr;
    #endif
    return esp_camera_fb_get();
}

CameraFrame Camera::captureJPEG() {
    CameraFrame frame = {nullptr, 0, false};
    
    #ifdef CAMERA_DISABLED
    return frame;
    #endif

    camera_fb_t* fb = capture();
    
    if (!fb) {
        _log->error("Camera capture failed");
        return frame;
    }
    
    if (fb->format == PIXFORMAT_JPEG) {
        // Already JPEG, just return the data
        frame.buf = fb->buf;
        frame.len = fb->len;
        frame.isAllocated = false;
        release(fb);
        return frame;
    }
    
    // Need to convert to JPEG
    uint8_t* jpeg_buf = nullptr;
    size_t jpeg_len = 0;
    bool converted = false;
    
    if (fb->format == PIXFORMAT_RGB565) {
        converted = fmt2jpg(fb->buf, fb->len, fb->width, fb->height, PIXFORMAT_RGB565, 80, &jpeg_buf, &jpeg_len);
    } else if (fb->format == PIXFORMAT_YUV422) {
        converted = fmt2jpg(fb->buf, fb->len, fb->width, fb->height, PIXFORMAT_YUV422, 80, &jpeg_buf, &jpeg_len);
    } else {
        _log->error("Unsupported pixel format: %d", fb->format);
    }
    
    esp_camera_fb_return(fb);
    
    if (!converted) {
        _log->error("JPEG conversion failed");
        if (jpeg_buf) free(jpeg_buf);
        return frame;
    }
    
    frame.buf = jpeg_buf;
    frame.len = jpeg_len;
    frame.isAllocated = true;
    return frame;
}

void Camera::release(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

void Camera::releaseFrame(CameraFrame& frame) {
    #ifdef CAMERA_DISABLED
    return;
    #endif
    if (frame.isAllocated && frame.buf) {
        free(frame.buf);
    }
    frame.buf = nullptr;
    frame.len = 0;
    frame.isAllocated = false;
}

esp_err_t Camera::setFrameSize(framesize_t size) {
    #ifdef CAMERA_DISABLED
    return ESP_ERR_CAMERA_NOT_DETECTED;
    #endif

    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) return ESP_FAIL;
    int result =  sensor->set_framesize(sensor, size);
    
    delay(1);
    return result;
}

esp_err_t Camera::setQuality(int quality) {
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) return ESP_FAIL;
    _quality = quality;
    int result = sensor->set_quality(sensor, quality);
    
    delay(1);
    return result;
}

esp_err_t Camera::setBrightness(int brightness) {
    #ifdef CAMERA_DISABLED
    return ESP_ERR_CAMERA_NOT_DETECTED;
    #endif

    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) return ESP_FAIL;
    int result = sensor->set_brightness(sensor, brightness);
    
    delay(1);
    return result;
}

esp_err_t Camera::setContrast(int contrast) {
    #ifdef CAMERA_DISABLED
    return ESP_ERR_CAMERA_NOT_DETECTED;
    #endif

    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) return ESP_FAIL;
    int result = sensor->set_contrast(sensor, contrast);
    
    delay(1);
    return result;
}

esp_err_t Camera::setSaturation(int saturation) {
    #ifdef CAMERA_DISABLED
    return ESP_ERR_CAMERA_NOT_DETECTED;
    #endif
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) return ESP_FAIL;
    int result = sensor->set_saturation(sensor, saturation);
    
    delay(1);
    return result;
}
