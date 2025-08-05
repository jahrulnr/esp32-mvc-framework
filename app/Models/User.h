#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <MVCFramework.h>

// DECLARE_MODEL(User, "users")

class User : public Model {
public:
    User() : Model("users") {}
    
    // Attributes
    String getName() const { return get("name"); }
    void setName(const String& name) { set("name", name); }
    
    String getEmail() const { return get("email"); }
    void setEmail(const String& email) { set("email", email); }
    
    String getPassword() const { return get("password"); }
    void setPassword(const String& password) { set("password", password); }
    
    bool isActive() const { return get("active") == "1"; }
    void setActive(bool active) { set("active", active ? "1" : "0"); }
    
    // Role management (0=system, 1=admin, 2=user)
    int getRole() const { return get("role").toInt(); }
    void setRole(int role) { set("role", String(role)); }
    String getRoleString() const;
    
    bool isSystem() const { return getRole() == 0; }
    bool isAdmin() const { return getRole() == 1; }
    bool isUser() const { return getRole() == 2; }
    
    // Permission checks
    bool canManageUsers() const { return isSystem() || isAdmin(); }
		bool canAccessAdmin() const { return isSystem() || isAdmin(); }
    bool canEditUser(const User* targetUser) const;
    bool canDeleteUser(const User* targetUser) const;
    bool canViewUser(const User* targetUser) const;
    
    String getCreatedAt() const { return get("created_at"); }
    String getUpdatedAt() const { return get("updated_at"); }
    
    // Methods
    bool authenticate(const String& password) const;
    void hashPassword(const String& password);
    bool hasRole(const String& role) const;
    
    // Static methods
    static User* findByEmail(const String& email);
    static std::vector<User*> active();
    static std::vector<User*> byRole(int role);
    static std::vector<User*> all();
    static bool canCurrentUserManage(const String& currentUserEmail, const String& targetUserEmail);
    
    // Validation
    bool validate() const;
    std::vector<String> getValidationErrors() const;
    
protected:
    void touch() override;
};

#endif
