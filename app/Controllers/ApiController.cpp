#include "ApiController.h"
#include "../Models/User.h"

Response ApiController::index(Request& request) {
    JsonDocument data;
    data["data"] = JsonArray();
    
    // Simulate fetching data
    for (int i = 1; i <= 5; i++) {
        JsonObject item = data["data"].add<JsonObject>();
        item["id"] = i;
        item["name"] = "Item " + String(i);
        item["value"] = i * 10;
        item["created_at"] = millis() - (i * 1000);
    }
    
    data["total"] = 5;
    data["page"] = request.input("page", "1").toInt();
    data["per_page"] = request.input("per_page", "10").toInt();
    
    return Response(request.getServerRequest())
        .json(data);
}

Response ApiController::show(Request& request) {
    String id = request.route("id");
    
    if (id.length() == 0) {
        return error(request.getServerRequest(), "ID parameter is required", 400);
    }
    
    JsonDocument data;
    data["id"] = id.toInt();
    data["name"] = "Item " + id;
    data["value"] = id.toInt() * 10;
    data["created_at"] = millis() - (id.toInt() * 1000);
    data["updated_at"] = millis();
    
    return Response(request.getServerRequest())
        .json(data);
}

Response ApiController::store(Request& request) {
    String name = request.input("name");
    String value = request.input("value");
    
    if (name.length() == 0) {
        return error(request.getServerRequest(), "Name is required", 400);
    }
    
    // Simulate creating new resource
    JsonDocument data;
    data["id"] = random(1000, 9999);
    data["name"] = name;
    data["value"] = value.length() > 0 ? value.toInt() : 0;
    data["created_at"] = millis();
    data["updated_at"] = millis();
    
    return Response(request.getServerRequest())
        .status(201)
        .json(data);
}

Response ApiController::update(Request& request) {
    String id = request.route("id");
    String name = request.input("name");
    String value = request.input("value");
    
    if (id.length() == 0) {
        return error(request.getServerRequest(), "ID parameter is required", 400);
    }
    
    // Simulate updating resource
    JsonDocument data;
    data["id"] = id.toInt();
    data["name"] = name.length() > 0 ? name : "Item " + id;
    data["value"] = value.length() > 0 ? value.toInt() : id.toInt() * 10;
    data["created_at"] = millis() - (id.toInt() * 1000);
    data["updated_at"] = millis();
    
    return Response(request.getServerRequest())
        .json(data);
}

Response ApiController::destroy(Request& request) {
    String id = request.route("id");
    
    if (id.length() == 0) {
        return error(request.getServerRequest(), "ID parameter is required", 400);
    }
    
    JsonDocument data;
    data["message"] = "Resource with ID " + id + " has been deleted";
    data["deleted_at"] = millis();
    
    return Response(request.getServerRequest())
        .json(data);
}

Response ApiController::status(Request& request) {
    JsonDocument data;
    data["status"] = "online";
    data["uptime"] = millis();
    data["free_heap"] = ESP.getFreeHeap();
    data["chip_id"] = ESP.getChipModel();
    data["flash_size"] = ESP.getFlashChipSize();
    data["cpu_freq"] = ESP.getCpuFreqMHz();
    
    return Response(request.getServerRequest())
        .json(data);
}

Response ApiController::health(Request& request) {
    JsonDocument data;
    data["healthy"] = true;
    data["checks"] = JsonObject();
    data["checks"]["memory"] = ESP.getFreeHeap() > 10000 ? "ok" : "warning";
    data["checks"]["uptime"] = millis() > 0 ? "ok" : "error";
    data["timestamp"] = millis();
    
    return Response(request.getServerRequest())
        .json(data);
}

Response ApiController::version(Request& request) {
    JsonDocument data;
    data["framework"] = "ESP32 MVC";
    data["version"] = "1.0.0";
    data["api_version"] = "v1";
    data["build"] = __DATE__ " " __TIME__;
    
    return Response(request.getServerRequest())
        .json(data);
}

Response ApiController::users(Request& request) {
    // Get all active users from CSV database
    std::vector<User*> users = User::active();
    
    JsonDocument data;
    data["users"] = JsonArray();
    
    for (User* user : users) {
        JsonObject userObj = data["users"].add<JsonObject>();
        userObj["id"] = user->get("id");
        userObj["name"] = user->getName();
        userObj["email"] = user->getEmail();
        userObj["role"] = user->get("role");
        userObj["active"] = user->isActive();
        userObj["created_at"] = user->getCreatedAt();
        userObj["updated_at"] = user->getUpdatedAt();
    }
    
    data["total"] = users.size();
    data["message"] = "Users retrieved from CSV database";
    
    // Clean up memory
    for (User* user : users) {
        delete user;
    }
    
    return Response(request.getServerRequest())
        .json(data);
}
