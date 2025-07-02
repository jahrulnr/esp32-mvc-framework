#include "User.h"
#include <Crypto.h>

bool User::authenticate(const String& password) const {
    // Simple password check (in production, use proper hashing)
    return getPassword() == password;
}

void User::hashPassword(const String& password) {
    // Simple hash (in production, use proper password hashing like bcrypt)
    // For now, just store as-is for demo purposes
    setPassword(password);
}

bool User::hasRole(const String& role) const {
    String userRole = get("role");
    return userRole == role;
}

User* User::findByEmail(const String& email) {
    if (!database) {
        return nullptr;
    }
    
    std::map<String, String> where;
    where["email"] = email;
    
    auto record = database->findWhere("users", where);
    if (record.empty()) {
        return nullptr;
    }
    
    User* user = new User();
    user->fill(record);
    user->syncOriginal();
    user->exists = true;
    
    return user;
}

std::vector<User*> User::active() {
    if (!database) {
        return std::vector<User*>();
    }
    
    std::map<String, String> where;
    where["active"] = "1";
    
    auto records = database->select("users", where);
    std::vector<User*> users;
    
    for (const auto& record : records) {
        User* user = new User();
        user->fill(record);
        user->syncOriginal();
        user->exists = true;
        users.push_back(user);
    }
    
    return users;
}

bool User::validate() const {
    return getValidationErrors().empty();
}

std::vector<String> User::getValidationErrors() const {
    std::vector<String> errors;
    
    if (getName().length() == 0) {
        errors.push_back("Name is required");
    }
    
    if (getEmail().length() == 0) {
        errors.push_back("Email is required");
    } else if (getEmail().indexOf('@') == -1) {
        errors.push_back("Email must be valid");
    }
    
    if (getPassword().length() < 6) {
        errors.push_back("Password must be at least 6 characters");
    }
    
    return errors;
}

void User::touch() {
    unsigned long now = millis();
    String timestamp = String(now);
    
    if (get("created_at").length() == 0) {
        set("created_at", timestamp);
    }
    set("updated_at", timestamp);
}
