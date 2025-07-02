#ifndef USER_MODEL_H
#define USER_MODEL_H

#include "../Framework/Database/Model.h"

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
    
    String getCreatedAt() const { return get("created_at"); }
    String getUpdatedAt() const { return get("updated_at"); }
    
    // Methods
    bool authenticate(const String& password) const;
    void hashPassword(const String& password);
    bool hasRole(const String& role) const;
    
    // Static methods
    static User* findByEmail(const String& email);
    static std::vector<User*> active();
    
    // Validation
    bool validate() const;
    std::vector<String> getValidationErrors() const;
    
protected:
    void touch() override;
};

#endif
