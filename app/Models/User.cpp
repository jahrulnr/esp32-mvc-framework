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

String User::getRoleString() const {
    int role = getRole();
    switch (role) {
        case 0: return "system";
        case 1: return "admin";
        case 2: return "user";
        default: return "unknown";
    }
}

bool User::canEditUser(const User* targetUser) const {
    if (!targetUser) return false;
    
    int myRole = getRole();
    int targetRole = targetUser->getRole();
    
    // System can edit anyone
    if (myRole == 0) return true;
    
    // Admin can edit users but not system or other admins
    if (myRole == 1 && targetRole == 2) return true;
    
    // Users can only edit themselves
    if (myRole == 2 && getEmail() == targetUser->getEmail()) return true;
    
    return false;
}

bool User::canDeleteUser(const User* targetUser) const {
    if (!targetUser) return false;
    
    int myRole = getRole();
    int targetRole = targetUser->getRole();
    
    // System can delete anyone except themselves
    if (myRole == 0 && getEmail() != targetUser->getEmail()) return true;
    
    // Admin can delete users but not system or other admins
    if (myRole == 1 && targetRole == 2) return true;
    
    return false;
}

bool User::canViewUser(const User* targetUser) const {
    if (!targetUser) return false;
    
    int myRole = getRole();
    
    // System and admin can view anyone
    if (myRole <= 1) return true;
    
    // Users can only view themselves
    if (myRole == 2 && getEmail() == targetUser->getEmail()) return true;
    
    return false;
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

std::vector<User*> User::byRole(int role) {
    if (!database) {
        return std::vector<User*>();
    }
    
    std::map<String, String> where;
    where["role"] = String(role);
    
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

std::vector<User*> User::all() {
    if (!database) {
        return std::vector<User*>();
    }
    
    auto records = database->select("users");
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

bool User::canCurrentUserManage(const String& currentUserEmail, const String& targetUserEmail) {
    User* currentUser = findByEmail(currentUserEmail);
    User* targetUser = findByEmail(targetUserEmail);
    
    if (!currentUser || !targetUser) {
        if (currentUser) delete currentUser;
        if (targetUser) delete targetUser;
        return false;
    }
    
    bool canManage = currentUser->canEditUser(targetUser);
    
    delete currentUser;
    delete targetUser;
    
    return canManage;
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
