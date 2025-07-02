#ifndef APPLICATION_H
#define APPLICATION_H

#include <Arduino.h>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include "../Routing/Router.h"
#include "ServiceContainer.h"
#include "Config.h"

class Application {
private:
    static Application* instance;
    std::unique_ptr<Router> router;
    std::unique_ptr<ServiceContainer> container;
    std::unique_ptr<Config> config;
    bool booted = false;
    
    Application() = default;

public:
    static Application* getInstance();
    
    void boot();
    void run();
    
    Router* getRouter() { return router.get(); }
    ServiceContainer* getContainer() { return container.get(); }
    Config* getConfig() { return config.get(); }
    
    template<typename T>
    void bind(const String& name, std::function<T*()> factory) {
        container->bind<T>(name, factory);
    }
    
    template<typename T>
    T* resolve(const String& name) {
        return container->resolve<T>(name);
    }
    
    void registerProviders();
    void registerMiddleware();
    void registerRoutes();
};

#endif
