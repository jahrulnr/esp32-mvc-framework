#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include <MVCFramework.h>
#include "../Models/User.h"

class UserController : public Controller {
public:
    // Web routes
    Response index(Request& request);     // List users (admin/system only)
    Response show(Request& request);      // Show user details
    Response create(Request& request);    // Show create user form
    Response store(Request& request);     // Create new user
    Response edit(Request& request);      // Show edit user form
    Response update(Request& request);    // Update user
    Response destroy(Request& request);   // Delete user
    
    // API routes
    Response apiIndex(Request& request);  // API: List users
    Response apiShow(Request& request);   // API: Show user
    Response apiStore(Request& request);  // API: Create user
    Response apiUpdate(Request& request); // API: Update user
    Response apiDestroy(Request& request);// API: Delete user
    
private:
    // Helper methods
    User* getCurrentUser(Request& request);
    bool hasPermission(User* currentUser, const String& action, User* targetUser = nullptr);
    Response unauthorized(Request& request, const String& message = "Unauthorized");
    Response notFound(Request& request, const String& message = "User not found");
    
    // Validation
    bool validateUserData(const JsonDocument& data, std::vector<String>& errors);
    bool isValidEmail(const String& email);
    bool isValidRole(int role);
};

#endif
