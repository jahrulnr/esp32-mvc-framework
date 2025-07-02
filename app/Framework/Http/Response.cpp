#include "Response.h"

Response::Response(AsyncWebServerRequest* req) 
    : request(req), statusCode(200), type("text/html") {
}

Response& Response::status(int code) {
    statusCode = code;
    return *this;
}

Response& Response::content(const String& body) {
    this->body = body;
    return *this;
}

Response& Response::html(const String& html) {
    body = html;
    type = "text/html";
    return *this;
}

Response& Response::text(const String& text) {
    body = text;
    type = "text/plain";
    return *this;
}

Response& Response::json(const JsonDocument& data) {
    String jsonString;
    serializeJson(data, jsonString);
    body = jsonString;
    type = "application/json";
    return *this;
}

Response& Response::json(const String& jsonString) {
    body = jsonString;
    type = "application/json";
    return *this;
}

Response& Response::header(const String& name, const String& value) {
    headers[name] = value;
    return *this;
}

Response& Response::contentType(const String& type) {
    this->type = type;
    return *this;
}

Response& Response::redirect(const String& url, int code) {
    statusCode = code;
    header("Location", url);
    return *this;
}

Response& Response::back() {
    // Get referer header and redirect there, or to home
    String referer = "";
    if (request->hasHeader("Referer")) {
        referer = request->getHeader("Referer")->value();
    }
    if (referer.length() == 0) {
        referer = "/";
    }
    return redirect(referer);
}

Response& Response::view(const String& template_name, const JsonDocument& data) {
    // TODO: Implement view rendering
    // For now, return simple HTML
    body = "<html><body><h1>View: " + template_name + "</h1></body></html>";
    type = "text/html";
    return *this;
}

Response& Response::file(const String& path) {
    // TODO: Implement file serving
    body = "File serving not implemented yet";
    return *this;
}

Response& Response::download(const String& path, const String& name) {
    String filename = name.length() > 0 ? name : path;
    header("Content-Disposition", "attachment; filename=\"" + filename + "\"");
    return file(path);
}

void Response::send() {
    if (!request) return;
    
    AsyncWebServerResponse* response = request->beginResponse(statusCode, type, body);
    
    // Add custom headers
    for (const auto& pair : headers) {
        response->addHeader(pair.first, pair.second);
    }
    
    request->send(response);
}
