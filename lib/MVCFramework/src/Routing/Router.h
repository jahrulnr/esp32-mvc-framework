#ifndef ROUTER_H
#define ROUTER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <map>
#include <vector>
#include <functional>

// Forward declarations
class Request;
class Response;
class Middleware;

struct Route {
    String method;
    String path;
    std::function<Response(Request&)> handler;
    std::vector<String> middleware;
    String name;
    std::map<String, String> parameters;
};

class Router {
private:
    AsyncWebServer* server;
    std::vector<Route> routes;
    std::map<String, std::shared_ptr<Middleware>> middlewares;
    String prefix;
    std::vector<String> middlewareStack;

public:
    Router(AsyncWebServer* webServer);
    
    // Route registration
    Router& get(const String& path, std::function<Response(Request&)> handler);
    Router& post(const String& path, std::function<Response(Request&)> handler);
    Router& put(const String& path, std::function<Response(Request&)> handler);
    Router& patch(const String& path, std::function<Response(Request&)> handler);
    Router& delete_(const String& path, std::function<Response(Request&)> handler);
    Router& any(const String& path, std::function<Response(Request&)> handler);
    
    // Route groups
    Router& group(const String& prefix, std::function<void(Router&)> routes);
    Router& middleware(const String& name);
    Router& middleware(const std::vector<String>& names);
    
    // Named routes
    Router& name(const String& routeName);
    String route(const String& name, const std::map<String, String>& parameters = {});
    
    // Controller routes
    Router& controller(const String& path, const String& controller);
    Router& resource(const String& path, const String& controller);
    
    // Middleware management
    void registerMiddleware(const String& name, std::shared_ptr<Middleware> middleware);
    
    // Route matching and execution
    bool match(const String& method, const String& path, AsyncWebServerRequest* request);
    void handleRequest(AsyncWebServerRequest* request);
    
    // Initialize routes on server
    void init();
    
private:
    Route& addRoute(const String& method, const String& path, std::function<Response(Request&)> handler);
    bool matchRoute(const Route& route, const String& method, const String& path, std::map<String, String>& params);
    String compilePath(const String& path);
    Response executeMiddleware(const std::vector<String>& middleware, Request& request, std::function<Response(Request&)> next);
};

#endif
