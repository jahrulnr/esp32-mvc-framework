#include "Router.h"
#include <regex>

Router::Router(AsyncWebServer* webServer) : server(webServer) {
}

Router& Router::get(const String& path, std::function<Response(Request&)> handler) {
    addRoute("GET", path, handler);
		return *this;
}

Router& Router::post(const String& path, std::function<Response(Request&)> handler) {
    addRoute("POST", path, handler);
		return *this;
}

Router& Router::put(const String& path, std::function<Response(Request&)> handler) {
    addRoute("PUT", path, handler);
		return *this;
}

Router& Router::patch(const String& path, std::function<Response(Request&)> handler) {
    addRoute("PATCH", path, handler);
		return *this;
}

Router& Router::delete_(const String& path, std::function<Response(Request&)> handler) {
    addRoute("DELETE", path, handler);
		return *this;
}

Router& Router::any(const String& path, std::function<Response(Request&)> handler) {
    addRoute("GET", path, handler);
    addRoute("POST", path, handler);
    addRoute("PUT", path, handler);
    addRoute("PATCH", path, handler);
    addRoute("DELETE", path, handler);
    return *this;
}

Router& Router::group(const String& groupPrefix, std::function<void(Router&)> routeFunc) {
    String oldPrefix = prefix;
    std::vector<String> oldMiddleware = middlewareStack;
    
    prefix = oldPrefix + groupPrefix;
    routeFunc(*this);
    
    prefix = oldPrefix;
    middlewareStack = oldMiddleware;
    return *this;
}

Router& Router::middleware(const String& name) {
    middlewareStack.push_back(name);
    return *this;
}

Router& Router::middleware(const std::vector<String>& names) {
    for (const String& name : names) {
        middlewareStack.push_back(name);
    }
    return *this;
}

Router& Router::name(const String& routeName) {
    if (!routes.empty()) {
        routes.back().name = routeName;
    }
    return *this;
}

String Router::route(const String& name, const std::map<String, String>& parameters) {
    for (const Route& route : routes) {
        if (route.name == name) {
            String path = route.path;
            // Replace parameters in path
            for (const auto& param : parameters) {
                path.replace("{" + param.first + "}", param.second);
            }
            return path;
        }
    }
    return "";
}

void Router::registerMiddleware(const String& name, std::shared_ptr<Middleware> middleware) {
    middlewares[name] = middleware;
}

Route& Router::addRoute(const String& method, const String& path, std::function<Response(Request&)> handler) {
    Route route;
    route.method = method;
    route.path = prefix + path;
    route.handler = handler;
    route.middleware = middlewareStack;
    
    routes.push_back(route);
    return routes.back();
}

bool Router::matchRoute(const Route& route, const String& method, const String& path, std::map<String, String>& params) {
    if (route.method != method && route.method != "ANY") {
        return false;
    }
    
    // Simple pattern matching for now
    // TODO: Implement proper regex matching for parameters
    if (route.path == path) {
        return true;
    }
    
    // Check for parameters in route
    if (route.path.indexOf('{') >= 0) {
        // Split both paths and compare segments
        std::vector<String> routeSegments;
        std::vector<String> pathSegments;
        
        // Simple split by '/'
        String routeCopy = route.path;
        String pathCopy = path;
        
        if (routeCopy.startsWith("/")) routeCopy = routeCopy.substring(1);
        if (pathCopy.startsWith("/")) pathCopy = pathCopy.substring(1);
        
        // Count segments
        int routeSegmentCount = 1;
        int pathSegmentCount = 1;
        
        for (int i = 0; i < routeCopy.length(); i++) {
            if (routeCopy.charAt(i) == '/') routeSegmentCount++;
        }
        
        for (int i = 0; i < pathCopy.length(); i++) {
            if (pathCopy.charAt(i) == '/') pathSegmentCount++;
        }
        
        if (routeSegmentCount != pathSegmentCount) {
            return false;
        }
        
        // Extract parameters (simplified)
        // This is a basic implementation
        return true;
    }
    
    return false;
}

void Router::init() {
    // Register all routes with the AsyncWebServer
    server->onNotFound([this](AsyncWebServerRequest* request) {
        handleRequest(request);
    });
    
    server->begin();
}

void Router::handleRequest(AsyncWebServerRequest* request) {
    String method;
    switch (request->method()) {
        case HTTP_GET: method = "GET"; break;
        case HTTP_POST: method = "POST"; break;
        case HTTP_PUT: method = "PUT"; break;
        case HTTP_DELETE: method = "DELETE"; break;
        case HTTP_PATCH: method = "PATCH"; break;
        default: method = "GET"; break;
    }
    
    String path = request->url();
    
    // Find matching route
    for (const Route& route : routes) {
        std::map<String, String> params;
        if (matchRoute(route, method, path, params)) {
            // Create request object
            Request req(request);
            
            // Set route parameters
            for (const auto& param : params) {
                req.setRouteParameter(param.first, param.second);
            }
            
            // Execute middleware chain
            Response response = executeMiddleware(route.middleware, req, route.handler);
            
            // Send response
            response.send();
            return;
        }
    }
    
    // No route found
    request->send(404, "text/plain", "Not Found");
}

Response Router::executeMiddleware(const std::vector<String>& middleware, Request& request, std::function<Response(Request&)> finalHandler) {
    if (middleware.empty()) {
        return finalHandler(request);
    }
    
    // Create middleware chain
    std::function<Response(Request&)> next = finalHandler;
    
    // Build chain in reverse order
    for (auto it = middleware.rbegin(); it != middleware.rend(); ++it) {
        const String& middlewareName = *it;
        auto middlewareIt = middlewares.find(middlewareName);
        
        if (middlewareIt != middlewares.end()) {
            auto middleware = middlewareIt->second;
            auto currentNext = next;
            
            next = [middleware, currentNext](Request& req) -> Response {
                return middleware->handle(req, currentNext);
            };
        }
    }
    
    return next(request);
}
