#include "AuthController.h"
#include "../Models/User.h"
#include <SPIFFS.h>

Response AuthController::showLogin(Request& request) {
		// Check if user is already authenticated
		String token = request.header("Authorization");
		if (token.length() > 0 && token.startsWith("Bearer ")) {
				String authToken = token.substring(7);
				if (verifyToken(authToken)) {
						return Response(request.getServerRequest())
								.redirect("/dashboard");
				}
		}
		
		// Serve login page from SPIFFS
		if (SPIFFS.exists("/views/login.html")) {
				File file = SPIFFS.open("/views/login.html", "r");
				String html = file.readString();
				file.close();
				
				return Response(request.getServerRequest())
						.html(html);
		}
		
		// Fallback if file doesn't exist
		JsonDocument data;
		data["title"] = "Login";
		data["action"] = "/login";
		data["redirect"] = request.get("redirect", "/dashboard");
		
		return Response(request.getServerRequest())
				.json(data);
}

Response AuthController::login(Request& request) {
		String email = request.input("email");
		String password = request.input("password");
		String redirect = request.input("redirect", "/dashboard");
		
		// Validate input
		if (email.length() == 0 || password.length() == 0) {
				JsonDocument error;
				error["success"] = false;
				error["message"] = "Email and password are required";
				
				return Response(request.getServerRequest())
						.status(400)
						.json(error);
		}
		
		// Validate credentials
		User* user = nullptr;
		if (!validateCredentials(email, password)) {
				JsonDocument error;
				error["success"] = false;
				error["message"] = "Invalid email or password";
				
				return Response(request.getServerRequest())
						.status(401)
						.json(error);
		}
		
		// Get user data from database
		user = User::findByEmail(email);
		
		// Generate JWT token
		String token = generateToken(email);
		
		JsonDocument response;
		response["success"] = true;
		response["message"] = "Login successful";
		response["token"] = token;
		response["redirect"] = redirect;
		response["user"]["email"] = user ? user->getEmail() : email;
		response["user"]["name"] = user ? user->getName() : "User";
		
		if (user) {
				delete user;
		}
		
		return Response(request.getServerRequest())
				.json(response);
}

Response AuthController::logout(Request& request) {
		// In a real implementation, you might want to blacklist the token
		JsonDocument response;
		response["success"] = true;
		response["message"] = "Logged out successfully";
		response["redirect"] = "/login";
		
		return Response(request.getServerRequest())
				.json(response);
}

Response AuthController::dashboard(Request& request) {
		// For web requests, let the client-side JavaScript handle authentication
		// The HTML page will check localStorage for token and redirect if needed
		
		// Serve dashboard page from SPIFFS
		if (SPIFFS.exists("/views/dashboard.html")) {
				File file = SPIFFS.open("/views/dashboard.html", "r");
				String html = file.readString();
				file.close();
				
				return Response(request.getServerRequest())
						.html(html);
		}
		
		// Fallback dashboard data (for JSON requests)
		JsonDocument data;
		data["title"] = "Dashboard";
		data["user"]["email"] = "demo@example.com";
		data["user"]["name"] = "Demo User";
		data["stats"] = JsonObject();
		data["stats"]["uptime"] = millis();
		data["stats"]["free_heap"] = ESP.getFreeHeap();
		
		return Response(request.getServerRequest())
				.json(data);
}

Response AuthController::showSignup(Request& request) {
		// Serve register page from SPIFFS
		if (SPIFFS.exists("/views/register.html")) {
				File file = SPIFFS.open("/views/register.html", "r");
				String html = file.readString();
				file.close();
				
				return Response(request.getServerRequest())
						.html(html);
		}
		
		// Fallback if file doesn't exist
		JsonDocument data;
		data["title"] = "Register";
		data["action"] = "/register";
		
		return Response(request.getServerRequest())
				.json(data);
}

Response AuthController::signup(Request& request) {
		String name = request.input("name");
		String email = request.input("email");
		String password = request.input("password");
		String confirmPassword = request.input("confirm_password");
		
		// Validate input
		if (name.length() == 0 || email.length() == 0 || password.length() == 0) {
				JsonDocument error;
				error["success"] = false;
				error["message"] = "All fields are required";
				
				return Response(request.getServerRequest())
						.status(400)
						.json(error);
		}
		
		if (password != confirmPassword) {
				JsonDocument error;
				error["success"] = false;
				error["message"] = "Passwords do not match";
				
				return Response(request.getServerRequest())
						.status(400)
						.json(error);
		}
		
		if (password.length() < 6) {
				JsonDocument error;
				error["success"] = false;
				error["message"] = "Password must be at least 6 characters";
				
				return Response(request.getServerRequest())
						.status(400)
						.json(error);
		}
		
		// Check if user already exists
		User* existingUser = User::findByEmail(email);
		if (existingUser != nullptr) {
				delete existingUser;
				JsonDocument error;
				error["success"] = false;
				error["message"] = "User with this email already exists";
				
				return Response(request.getServerRequest())
						.status(400)
						.json(error);
		}
		
		// Create new user
		User* newUser = new User();
		newUser->setName(name);
		newUser->setEmail(email);
		newUser->hashPassword(password); // This will just store the password as-is for demo
		newUser->setActive(true);
		newUser->set("role", "user");
		newUser->set("created_at", "2025-07-03 " + String(millis()));
		newUser->set("updated_at", "2025-07-03 " + String(millis()));
		
		// Save to database
		if (!newUser->save()) {
				delete newUser;
				JsonDocument error;
				error["success"] = false;
				error["message"] = "Failed to create user account";
				
				return Response(request.getServerRequest())
						.status(500)
						.json(error);
		}
		
		delete newUser;
		
		JsonDocument response;
		response["success"] = true;
		response["message"] = "Registration successful! Please login.";
		response["redirect"] = "/login";
		
		return Response(request.getServerRequest())
				.json(response);
}

bool AuthController::validateCredentials(const String& email, const String& password) {
		// Find user by email in CSV database
		User* user = User::findByEmail(email);
		
		if (user == nullptr) {
				return false; // User not found
		}
		
		// Check if user is active
		if (!user->isActive()) {
				delete user;
				return false; // User is not active
		}
		
		// Authenticate with password
		bool isValid = user->authenticate(password);
		
		delete user;
		return isValid;
}

String AuthController::generateToken(const String& email) {
		// Simple token generation - in production use proper JWT library
		// For demo, encode email in the token for easy extraction
		String token = "cozmo_token_" + email + "_" + String(millis());
		// In real implementation, use proper JWT encoding with signature
		return token;
}

bool AuthController::verifyToken(const String& token) {
		// Simple token verification - in production use proper JWT verification
		return token.startsWith("cozmo_token_") && token.length() > 20;
}

String AuthController::extractEmailFromToken(const String& token) {
		// Simple email extraction - in production parse JWT payload
		if (token.indexOf("cozmo_token_") == 0) {
				// Extract email from token format: cozmo_token_email_timestamp
				int firstUnderscore = token.indexOf('_', 11); // After "cozmo_token_"
				int lastUnderscore = token.lastIndexOf('_');
				
				if (firstUnderscore != -1 && lastUnderscore != -1 && firstUnderscore != lastUnderscore) {
						String email = token.substring(11, lastUnderscore); // Extract email part\
						return email;
				}
		}
		return "";
}

// Static helper methods for other controllers
String AuthController::getCurrentUserEmail(Request& request) {
		String token = request.header("Authorization");
		
		if (token.length() == 0) {
				return "";
		}
		
		if (token.startsWith("Bearer ")) {
				token = token.substring(7);
		}
		
		// Simple token verification - in production use proper JWT
		if (!token.startsWith("cozmo_token_")) {
				return "";
		}
		
		// Extract email from token format: cozmo_token_email_timestamp
		int tokenPrefix = String("cozmo_token_").length();
		int lastUnderscore = token.lastIndexOf('_');
		
		if (lastUnderscore != -1 && token.indexOf("@") > 0) {
				String email = token.substring(tokenPrefix, lastUnderscore); // Extract email part
				return email;
		}
		
		return "";
}

User* AuthController::getCurrentUser(Request& request) {
		String email = getCurrentUserEmail(request);

		if (email.length() == 0) {
				return nullptr;
		}
		
		User* user = User::findByEmail(email);
		return user;
}

Response AuthController::getUserInfo(Request& request) {
	// Use the same authentication logic as other methods
	User* user = getCurrentUser(request);
	if (user == nullptr) {
		JsonDocument error;
		error["success"] = false;
		error["message"] = "Authentication required or user not found";
		
		return Response(request.getServerRequest())
			.status(401)
			.json(error);
	}
	
	// Return user info
	JsonDocument response;
	response["success"] = true;
	response["user"]["email"] = user->getEmail();
	response["user"]["name"] = user->getName();
	response["user"]["role"] = user->get("role");
	response["user"]["roleLevel"] = user->getRoleString();
	response["user"]["permissions"]["canManageUsers"] = user->canManageUsers();
	response["user"]["permissions"]["canAccessAdmin"] = user->canAccessAdmin();
	response["user"]["active"] = user->isActive();
	response["user"]["created_at"] = user->getCreatedAt();
	
	delete user;
	
	return Response(request.getServerRequest())
		.json(response);
}
