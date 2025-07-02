#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include "../Framework/Http/Controller.h"

class AuthController : public Controller {
public:
    // GET /login - Show login form
    Response showLogin(Request& request);
    
    // POST /login - Process login
    Response login(Request& request);
    
    // POST /logout - Process logout
    Response logout(Request& request);
    
    // GET /dashboard - Show dashboard (requires auth)
    Response dashboard(Request& request);
    
    // GET /register - Show registration form
    Response showSignup(Request& request);
    
    // POST /register - Process registration
    Response signup(Request& request);
    
    // Static helper methods for other controllers
    static String getCurrentUserEmail(Request& request);
    static class User* getCurrentUser(Request& request);
    
    // API method for getting current user info
    Response getUserInfo(Request& request);
    
private:
    bool validateCredentials(const String& email, const String& password);
    String generateToken(const String& email);
    bool verifyToken(const String& token);
    String extractEmailFromToken(const String& token);
};

#endif
