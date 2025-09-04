#ifndef APPLICATION_H
#define APPLICATION_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <LittleFS.h>
#include <map>
#include <vector>
#include <functional>
#include <memory>

// Forward declarations
class Config;
class ServiceContainer;
class Router;

class Application {
private:
    static Application* instance;
    std::unique_ptr<Config> config;
    std::unique_ptr<ServiceContainer> container;
    std::unique_ptr<Router> router;
    bool booted = false;
    const char* deviceName = "PioDevice-ESP32Cam";
    fs::FS& _storageType = LittleFS;
    
    Application() = default;

public:
    static Application* getInstance(fs::FS& storageType = LittleFS);
    
    void boot();
    void run();
    
    void setDeviceName(const char* value) { deviceName = value; }
    const char* getDeviceName() const { return deviceName; }
    Router* getRouter() { return router.get(); }
    ServiceContainer* getContainer() { return container.get(); }
    Config* getConfig() { return config.get(); }
    
    // Template methods - implementation in ApplicationTemplates.h
    template<typename T>
    void bind(const String& name, std::function<T*()> factory);
    
    template<typename T>
    T* resolve(const String& name);
    
    void registerProviders();
    void registerMiddleware();
    void registerRoutes();
};

#endif
