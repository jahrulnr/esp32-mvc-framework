#ifndef RESPONSE_H
#define RESPONSE_H

#include <Arduino.h>
#include <map>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class Response {
private:
    AsyncWebServerRequest* request;
    String body;
    String type;
    int statusCode;
    std::map<String, String> headers;

public:
    Response(AsyncWebServerRequest* req);
    
    // Status codes
    Response& status(int code);
    Response& ok() { return status(200); }
    Response& created() { return status(201); }
    Response& notFound() { return status(404); }
    Response& unauthorized() { return status(401); }
    Response& forbidden() { return status(403); }
    Response& internalServerError() { return status(500); }
    
    // Content
    Response& content(const String& body);
    Response& html(const String& html);
    Response& text(const String& text);
    Response& json(const JsonDocument& data);
    Response& json(const String& jsonString);
    
    // Headers
    Response& header(const String& name, const String& value);
    Response& contentType(const String& type);
    
    // Redirects
    Response& redirect(const String& url, int code = 302);
    Response& back();
    
    // Views
    Response& view(const String& template_name, const JsonDocument& data = JsonDocument());
    
    // File responses
    Response& file(const String& path);
    Response& download(const String& path, const String& name = "");
    
    // Send the response
    void send();
    
    // Getters
    int getStatusCode() const { return statusCode; }
    String getContent() const { return body; }
    String getContentType() const { return type; }
};

#endif
