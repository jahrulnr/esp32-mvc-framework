#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <MVCFramework.h>
#include "Routes/routes.h"
#include "Controllers/CameraController.h"
#include "Models/Configuration.h"
#include "Servo.h"
#include "ServoConfig.h"

// WiFi credentials
extern const char* ssid;
extern const char* password;

extern String deviceName;
extern Application* app;
extern CsvDatabase* database;

void wifiMonitorTask(void* parameter);
void setupTasks();
void setupServos();

#endif