#include "HomeController.h"
#include <SPIFFS.h>

Response HomeController::index(Request& request) {
    // Try to serve index.html from SPIFFS
    if (SPIFFS.exists("/views/index.html")) {
        File file = SPIFFS.open("/views/index.html", "r");
        String html = file.readString();
        file.close();
        
        return Response(request.getServerRequest())
            .html(html);
    }
    
    // Fallback to JSON response
    JsonDocument data;
    data["title"] = "Welcome to ESP32 MVC";
    data["message"] = "Your MVC framework is working!";
    data["timestamp"] = millis();
    
    return Response(request.getServerRequest())
        .json(data);
}

Response HomeController::about(Request& request) {
    JsonDocument data;
    data["framework"] = "ESP32 MVC";
    data["version"] = "1.0.0";
    data["author"] = "ESP32 Developer";
    data["description"] = "A Laravel-inspired MVC framework for ESP32";
    
    return Response(request.getServerRequest())
        .json(data);
}

Response HomeController::contact(Request& request) {
    if (request.isPost()) {
        String name = request.input("name");
        String email = request.input("email");
        String message = request.input("message");
        
        if (name.length() == 0 || email.length() == 0 || message.length() == 0) {
            return error(request.getServerRequest(), "All fields are required", 400);
        }
        
        // Process contact form (save to database, send email, etc.)
        JsonDocument response;
        response["success"] = true;
        response["message"] = "Thank you for your message, " + name + "!";
        
        return Response(request.getServerRequest())
            .json(response);
    }
    
    // Return contact form data for GET request
    JsonDocument data;
    data["title"] = "Contact Us";
    data["fields"] = JsonArray();
    data["fields"].add("name");
    data["fields"].add("email");
    data["fields"].add("message");
    
    return Response(request.getServerRequest())
        .json(data);
}
