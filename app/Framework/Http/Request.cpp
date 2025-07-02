#include "Request.h"

Request::Request(AsyncWebServerRequest* request) : serverRequest(request) {
    // Extract headers
    int headerCount = request->headers();
    for (int i = 0; i < headerCount; i++) {
        const AsyncWebHeader* header = request->getHeader(i);
        headers[header->name()] = header->value();
    }
    
    // Extract parameters
    int paramCount = request->params();
    for (int i = 0; i < paramCount; i++) {
        const AsyncWebParameter* param = request->getParam(i);
        parameters[param->name()] = param->value();
    }
}

String Request::method() const {
    if (!serverRequest) return "";
    
    switch (serverRequest->method()) {
        case HTTP_GET: return "GET";
        case HTTP_POST: return "POST";
        case HTTP_PUT: return "PUT";
        case HTTP_DELETE: return "DELETE";
        case HTTP_PATCH: return "PATCH";
        case HTTP_HEAD: return "HEAD";
        case HTTP_OPTIONS: return "OPTIONS";
        default: return "UNKNOWN";
    }
}

String Request::url() const {
    if (!serverRequest) return "";
    return serverRequest->url();
}

String Request::path() const {
    String fullUrl = url();
    int queryIndex = fullUrl.indexOf('?');
    if (queryIndex >= 0) {
        return fullUrl.substring(0, queryIndex);
    }
    return fullUrl;
}

String Request::query() const {
    String fullUrl = url();
    int queryIndex = fullUrl.indexOf('?');
    if (queryIndex >= 0) {
        return fullUrl.substring(queryIndex + 1);
    }
    return "";
}

String Request::input(const String& key, const String& defaultValue) const {
    return get(key, defaultValue);
}

String Request::get(const String& key, const String& defaultValue) const {
    auto it = parameters.find(key);
    if (it != parameters.end()) {
        return it->second;
    }
    return defaultValue;
}

String Request::post(const String& key, const String& defaultValue) const {
    if (isPost()) {
        return get(key, defaultValue);
    }
    return defaultValue;
}

bool Request::has(const String& key) const {
    return parameters.find(key) != parameters.end();
}

String Request::header(const String& name, const String& defaultValue) const {
    auto it = headers.find(name);
    if (it != headers.end()) {
        return it->second;
    }
    return defaultValue;
}

bool Request::hasHeader(const String& name) const {
    return headers.find(name) != headers.end();
}

bool Request::hasFile(const String& name) const {
    // TODO: Implement file upload support
    return false;
}

bool Request::filled(const String& key) const {
    String value = get(key);
    return value.length() > 0;
}

JsonDocument Request::json() const {
    JsonDocument doc;
    if (body.length() > 0) {
        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            Serial.println("Failed to parse JSON body");
        }
    }
    return doc;
}

bool Request::wantsJson() const {
    String accept = header("Accept");
    String contentType = header("Content-Type");
    
    return accept.indexOf("application/json") >= 0 || 
           contentType.indexOf("application/json") >= 0;
}

String Request::ip() const {
    if (!serverRequest) return "";
    return serverRequest->client()->remoteIP().toString();
}

String Request::userAgent() const {
    return header("User-Agent");
}

void Request::setRouteParameter(const String& key, const String& value) {
    parameters[key] = value;
}

String Request::route(const String& key, const String& defaultValue) const {
    return get(key, defaultValue);
}
