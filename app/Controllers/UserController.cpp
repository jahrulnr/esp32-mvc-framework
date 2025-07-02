#include "UserController.h"
#include "AuthController.h"
#include <SPIFFS.h>

// Web Routes Implementation

Response UserController::index(Request& request) {
    User* currentUser = getCurrentUser(request);
    if (!currentUser || !currentUser->canManageUsers()) {
        if (currentUser) delete currentUser;
        return unauthorized(request, "You don't have permission to view users");
    }
    
    // Get users based on current user role
    std::vector<User*> users;
    if (currentUser->isSystem()) {
        users = User::all(); // System can see all users
    } else if (currentUser->isAdmin()) {
        // Admin can see users and other admins, but not system users
        auto allUsers = User::all();
        for (User* user : allUsers) {
            if (user->getRole() >= 1) { // Admin and regular users
                users.push_back(user);
            } else {
                delete user; // Clean up system users
            }
        }
    }
    
    // Serve users management page from SPIFFS
    if (SPIFFS.exists("/views/users.html")) {
        File file = SPIFFS.open("/views/users.html", "r");
        String html = file.readString();
        file.close();
        
        delete currentUser;
        // Clean up users vector
        for (User* user : users) {
            delete user;
        }
        
        return Response(request.getServerRequest())
            .html(html);
    }
    
    // Fallback JSON response
    JsonDocument data;
    data["users"] = JsonArray();
    
    for (User* user : users) {
        JsonObject userObj = data["users"].add<JsonObject>();
        userObj["id"] = user->get("id");
        userObj["name"] = user->getName();
        userObj["email"] = user->getEmail();
        userObj["role"] = user->getRole();
        userObj["role_string"] = user->getRoleString();
        userObj["active"] = user->isActive();
        userObj["created_at"] = user->getCreatedAt();
        userObj["updated_at"] = user->getUpdatedAt();
    }
    
    data["current_user"]["role"] = currentUser->getRole();
    data["current_user"]["role_string"] = currentUser->getRoleString();
    
    // Clean up
    delete currentUser;
    for (User* user : users) {
        delete user;
    }
    
    return Response(request.getServerRequest())
        .json(data);
}

Response UserController::show(Request& request) {
    String userId = request.route("id");
    if (userId.length() == 0) {
        return notFound(request, "User ID is required");
    }
    
    User* currentUser = getCurrentUser(request);
    if (!currentUser) {
        return unauthorized(request, "Authentication required");
    }
    
    User* targetUser = (User*) User::find("users", userId);
    if (!targetUser) {
        delete currentUser;
        return notFound(request, "User not found");
    }
    
    if (!currentUser->canViewUser(targetUser)) {
        delete currentUser;
        delete targetUser;
        return unauthorized(request, "You don't have permission to view this user");
    }
    
    JsonDocument data;
    data["user"]["id"] = targetUser->get("id");
    data["user"]["name"] = targetUser->getName();
    data["user"]["email"] = targetUser->getEmail();
    data["user"]["role"] = targetUser->getRole();
    data["user"]["role_string"] = targetUser->getRoleString();
    data["user"]["active"] = targetUser->isActive();
    data["user"]["created_at"] = targetUser->getCreatedAt();
    data["user"]["updated_at"] = targetUser->getUpdatedAt();
    
    data["permissions"]["can_edit"] = currentUser->canEditUser(targetUser);
    data["permissions"]["can_delete"] = currentUser->canDeleteUser(targetUser);
    
    delete currentUser;
    delete targetUser;
    
    return Response(request.getServerRequest())
        .json(data);
}

Response UserController::store(Request& request) {
    User* currentUser = getCurrentUser(request);
    if (!currentUser || !currentUser->canManageUsers()) {
        if (currentUser) delete currentUser;
        return unauthorized(request, "You don't have permission to create users");
    }
    
    String name = request.input("name");
    String email = request.input("email");
    String password = request.input("password");
    int role = request.input("role", "2").toInt(); // Default to regular user
    bool active = request.input("active", "1") == "1";
    
    // Validation
    std::vector<String> errors;
    
    if (name.length() == 0) {
        errors.push_back("Name is required");
    }
    
    if (email.length() == 0) {
        errors.push_back("Email is required");
    } else if (!isValidEmail(email)) {
        errors.push_back("Invalid email format");
    } else {
        // Check if email already exists
        User* existingUser = User::findByEmail(email);
        if (existingUser) {
            delete existingUser;
            errors.push_back("Email already exists");
        }
    }
    
    if (password.length() < 6) {
        errors.push_back("Password must be at least 6 characters");
    }
    
    if (!isValidRole(role)) {
        errors.push_back("Invalid role");
    }
    
    // Role permission check
    if (currentUser->isAdmin() && role <= 1) {
        errors.push_back("Admins cannot create system or admin users");
    }
    
    if (!errors.empty()) {
        JsonDocument response;
        response["success"] = false;
        response["errors"] = JsonArray();
        for (const String& error : errors) {
            response["errors"].add(error);
        }
        
        delete currentUser;
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    // Create new user
    User* newUser = new User();
    newUser->setName(name);
    newUser->setEmail(email);
    newUser->hashPassword(password);
    newUser->setRole(role);
    newUser->setActive(active);
    newUser->set("created_at", "2025-07-03 " + String(millis()));
    newUser->set("updated_at", "2025-07-03 " + String(millis()));
    
    if (!newUser->save()) {
        delete currentUser;
        delete newUser;
        
        JsonDocument response;
        response["success"] = false;
        response["message"] = "Failed to create user";
        
        return Response(request.getServerRequest())
            .status(500)
            .json(response);
    }
    
    JsonDocument response;
    response["success"] = true;
    response["message"] = "User created successfully";
    response["user"]["id"] = newUser->get("id");
    response["user"]["name"] = newUser->getName();
    response["user"]["email"] = newUser->getEmail();
    response["user"]["role"] = newUser->getRole();
    response["user"]["role_string"] = newUser->getRoleString();
    
    delete currentUser;
    delete newUser;
    
    return Response(request.getServerRequest())
        .json(response);
}

Response UserController::update(Request& request) {
    String userId = request.route("id");
    if (userId.length() == 0) {
        return notFound(request, "User ID is required");
    }
    
    User* currentUser = getCurrentUser(request);
    if (!currentUser) {
        return unauthorized(request, "Authentication required");
    }
    
    User* targetUser = (User*) User::find("users", userId);
    if (!targetUser) {
        delete currentUser;
        return notFound(request, "User not found");
    }
    
    if (!currentUser->canEditUser(targetUser)) {
        delete currentUser;
        delete targetUser;
        return unauthorized(request, "You don't have permission to edit this user");
    }
    
    // Get update data
    String name = request.input("name", targetUser->getName());
    String email = request.input("email", targetUser->getEmail());
    String password = request.input("password");
    String roleStr = request.input("role", String(targetUser->getRole()));
    int role = roleStr.toInt();
    bool active = request.input("active", targetUser->isActive() ? "1" : "0") == "1";
    
    // Validation
    std::vector<String> errors;
    
    if (name.length() == 0) {
        errors.push_back("Name is required");
    }
    
    if (email.length() == 0) {
        errors.push_back("Email is required");
    } else if (!isValidEmail(email)) {
        errors.push_back("Invalid email format");
    } else if (email != targetUser->getEmail()) {
        // Check if new email already exists
        User* existingUser = User::findByEmail(email);
        if (existingUser) {
            delete existingUser;
            errors.push_back("Email already exists");
        }
    }
    
    if (password.length() > 0 && password.length() < 6) {
        errors.push_back("Password must be at least 6 characters");
    }
    
    if (!isValidRole(role)) {
        errors.push_back("Invalid role");
    }
    
    // Role permission check
    if (currentUser->isAdmin() && role <= 1) {
        errors.push_back("Admins cannot set system or admin roles");
    }
    
    if (!errors.empty()) {
        JsonDocument response;
        response["success"] = false;
        response["errors"] = JsonArray();
        for (const String& error : errors) {
            response["errors"].add(error);
        }
        
        delete currentUser;
        delete targetUser;
        return Response(request.getServerRequest())
            .status(400)
            .json(response);
    }
    
    // Update user
    targetUser->setName(name);
    targetUser->setEmail(email);
    if (password.length() > 0) {
        targetUser->hashPassword(password);
    }
    targetUser->setRole(role);
    targetUser->setActive(active);
    targetUser->set("updated_at", "2025-07-03 " + String(millis()));
    
    if (!targetUser->save()) {
        delete currentUser;
        delete targetUser;
        
        JsonDocument response;
        response["success"] = false;
        response["message"] = "Failed to update user";
        
        return Response(request.getServerRequest())
            .status(500)
            .json(response);
    }
    
    JsonDocument response;
    response["success"] = true;
    response["message"] = "User updated successfully";
    response["user"]["id"] = targetUser->get("id");
    response["user"]["name"] = targetUser->getName();
    response["user"]["email"] = targetUser->getEmail();
    response["user"]["role"] = targetUser->getRole();
    response["user"]["role_string"] = targetUser->getRoleString();
    response["user"]["active"] = targetUser->isActive();
    
    delete currentUser;
    delete targetUser;
    
    return Response(request.getServerRequest())
        .json(response);
}

Response UserController::destroy(Request& request) {
    String userId = request.route("id");
    if (userId.length() == 0) {
        return notFound(request, "User ID is required");
    }
    
    User* currentUser = getCurrentUser(request);
    if (!currentUser) {
        return unauthorized(request, "Authentication required");
    }
    
    User* targetUser = (User*) User::find("users", userId);
    if (!targetUser) {
        delete currentUser;
        return notFound(request, "User not found");
    }
    
    if (!currentUser->canDeleteUser(targetUser)) {
        delete currentUser;
        delete targetUser;
        return unauthorized(request, "You don't have permission to delete this user");
    }
    
    String targetEmail = targetUser->getEmail();
    
    if (!targetUser->delete_()) {
        delete currentUser;
        delete targetUser;
        
        JsonDocument response;
        response["success"] = false;
        response["message"] = "Failed to delete user";
        
        return Response(request.getServerRequest())
            .status(500)
            .json(response);
    }
    
    JsonDocument response;
    response["success"] = true;
    response["message"] = "User '" + targetEmail + "' deleted successfully";
    
    delete currentUser;
    delete targetUser;
    
    return Response(request.getServerRequest())
        .json(response);
}

// Helper Methods

User* UserController::getCurrentUser(Request& request) {
    return AuthController::getCurrentUser(request);
}

bool UserController::hasPermission(User* currentUser, const String& action, User* targetUser) {
    if (!currentUser) return false;
    
    if (action == "manage") {
        return currentUser->canManageUsers();
    } else if (action == "view" && targetUser) {
        return currentUser->canViewUser(targetUser);
    } else if (action == "edit" && targetUser) {
        return currentUser->canEditUser(targetUser);
    } else if (action == "delete" && targetUser) {
        return currentUser->canDeleteUser(targetUser);
    }
    
    return false;
}

Response UserController::unauthorized(Request& request, const String& message) {
    JsonDocument response;
    response["success"] = false;
    response["message"] = message;
    
    return Response(request.getServerRequest())
        .status(403)
        .json(response);
}

Response UserController::notFound(Request& request, const String& message) {
    JsonDocument response;
    response["success"] = false;
    response["message"] = message;
    
    return Response(request.getServerRequest())
        .status(404)
        .json(response);
}

bool UserController::isValidEmail(const String& email) {
    return email.indexOf('@') > 0 && email.indexOf('.') > email.indexOf('@');
}

bool UserController::isValidRole(int role) {
    return role >= 0 && role <= 2;
}

// API Routes (same logic as web routes but focused on JSON responses)

Response UserController::apiIndex(Request& request) {
    User* currentUser = getCurrentUser(request);
    if (!currentUser || !currentUser->canManageUsers()) {
        if (currentUser) delete currentUser;
        return unauthorized(request, "You don't have permission to view users");
    }
    
    // Get users based on current user role
    std::vector<User*> users;
    if (currentUser->isSystem()) {
        users = User::all(); // System can see all users
    } else if (currentUser->isAdmin()) {
        // Admin can see users and other admins, but not system users
        auto allUsers = User::all();
        for (User* user : allUsers) {
            if (user->getRole() >= 1) { // Admin and regular users
                users.push_back(user);
            } else {
                delete user; // Clean up system users
            }
        }
    }
    
    // Always return JSON for API endpoints
    JsonDocument data;
    data["success"] = true;
    data["users"] = JsonArray();
    
    for (User* user : users) {
        JsonObject userObj = data["users"].add<JsonObject>();
        userObj["id"] = user->get("id");
        userObj["name"] = user->getName();
        userObj["email"] = user->getEmail();
        userObj["role"] = user->getRole();
        userObj["role_string"] = user->getRoleString();
        userObj["active"] = user->isActive();
        userObj["created_at"] = user->getCreatedAt();
        userObj["updated_at"] = user->getUpdatedAt();
    }
    
    data["current_user"]["role"] = currentUser->getRole();
    data["current_user"]["role_string"] = currentUser->getRoleString();
    data["current_user"]["email"] = currentUser->getEmail();
    data["current_user"]["name"] = currentUser->getName();
    
    // Clean up
    delete currentUser;
    for (User* user : users) {
        delete user;
    }
    
    return Response(request.getServerRequest())
        .json(data);
}

Response UserController::apiShow(Request& request) {
    String userId = request.route("id");
    if (userId.length() == 0) {
        JsonDocument error;
        error["success"] = false;
        error["message"] = "User ID is required";
        
        return Response(request.getServerRequest())
            .status(400)
            .json(error);
    }
    
    User* currentUser = getCurrentUser(request);
    if (!currentUser) {
        JsonDocument error;
        error["success"] = false;
        error["message"] = "Authentication required";
        
        return Response(request.getServerRequest())
            .status(401)
            .json(error);
    }
    
    User* targetUser = (User*) User::find("users", userId);
    if (!targetUser) {
        delete currentUser;
        JsonDocument error;
        error["success"] = false;
        error["message"] = "User not found";
        
        return Response(request.getServerRequest())
            .status(404)
            .json(error);
    }
    
    if (!currentUser->canViewUser(targetUser)) {
        delete currentUser;
        delete targetUser;
        JsonDocument error;
        error["success"] = false;
        error["message"] = "You don't have permission to view this user";
        
        return Response(request.getServerRequest())
            .status(403)
            .json(error);
    }
    
    JsonDocument data;
    data["success"] = true;
    data["user"]["id"] = targetUser->get("id");
    data["user"]["name"] = targetUser->getName();
    data["user"]["email"] = targetUser->getEmail();
    data["user"]["role"] = targetUser->getRole();
    data["user"]["role_string"] = targetUser->getRoleString();
    data["user"]["active"] = targetUser->isActive();
    data["user"]["created_at"] = targetUser->getCreatedAt();
    data["user"]["updated_at"] = targetUser->getUpdatedAt();
    
    data["permissions"]["can_edit"] = currentUser->canEditUser(targetUser);
    data["permissions"]["can_delete"] = currentUser->canDeleteUser(targetUser);
    
    delete currentUser;
    delete targetUser;
    
    return Response(request.getServerRequest())
        .json(data);
}

Response UserController::apiStore(Request& request) {
    return store(request); // Store already returns JSON
}

Response UserController::apiUpdate(Request& request) {
    return update(request); // Update already returns JSON
}

Response UserController::apiDestroy(Request& request) {
    return destroy(request); // Destroy already returns JSON
}
