#include "routes.h"
#include "../Controllers/CameraController.h"
#include "../Controllers/SystemController.h"
#include "../Controllers/ServoController.h"
#include <SPIFFS.h>

void registerWebRoutes(Router* router) {
		AuthController* authController = new AuthController();

		// Single-page application route
		router->get("/", [](Request& request) -> Response {
				// Serve the app.html as the main entry point
				if (SPIFFS.exists("/views/app.html")) {
						
						return Response(request.getServerRequest())
								.file("/views/app.html");
				}
				
				return Response(request.getServerRequest())
						.status(404);
		}).name("app");
		
		// Authentication routes
		router->get("/login", [](Request& request) -> Response {
				// Redirect to the main app with login hash
				return Response(request.getServerRequest())
						.redirect("/#login");
		}).name("login.show");
		
		router->post("/login", [authController](Request& request) -> Response {
				return authController->login(request);
		}).name("login");
		
		router->post("/logout", [authController](Request& request) -> Response {
				return authController->logout(request);
		}).name("logout");
		
		// Protected routes (client-side auth check)
		router->get("/dashboard", [](Request& request) -> Response {
				// Redirect to the main app with dashboard hash
				return Response(request.getServerRequest())
						.redirect("/#dashboard");
		}).name("dashboard");
		
		// Static file serving for CSS, JS, and other assets
		router->get("/assets/{file}", [](Request& request) -> Response {
				String file = request.route("file");
				String path = "/assets/" + file;
				
				return Response(request.getServerRequest())
						.file(path);
		}).name("assets");


		router->get("/favicon.ico", [](Request& request) -> Response {
				return Response(request.getServerRequest())
						.file("/favicon.ico");
		});
}

void registerApiRoutes(Router* router) {
		// API routes with middleware
		router->group("/api/v1", [&](Router& api) {
				api.middleware({"cors", "json", "ratelimit"});
				
				// Auth routes for user info (register first to avoid conflicts)
				api.group("/auth", [&](Router& auth) {
						AuthController* authController = new AuthController();
						
						auth.get("/user", [authController](Request& request) -> Response {
								return authController->getUserInfo(request);
						}).name("api.auth.user");
						
						auth.post("/password", [authController](Request& request) -> Response {
								// Password update endpoint (not implemented yet)
								JsonDocument response;
								response["success"] = false;
								response["message"] = "Password update not implemented yet";
								
								return Response(request.getServerRequest())
										.status(200)
										.json(response);
						}).name("api.auth.password");
				});
				
				// Admin routes
				api.group("/admin", [&](Router& admin) {
						admin.middleware({"auth", "admin", "json"}); // Middleware to check if user is admin
						
						AuthController* authController = new AuthController();
						
						admin.get("/users", [authController](Request& request) -> Response {
								// Get all users (not implemented yet)
								JsonDocument response;
								response["success"] = true;
								response["users"] = JsonArray();
								
								// Return demo data
								JsonObject user1 = response["users"].add<JsonObject>();
								user1["id"] = 1;
								user1["name"] = "Admin User";
								user1["username"] = "admin";
								user1["role"] = "admin";
								user1["active"] = true;
								
								return Response(request.getServerRequest())
										.status(200)
										.json(response);
						}).name("api.admin.users");
				});
				
				// Camera routes
				api.group("/camera", [&](Router& camera) {
						camera.middleware({"auth", "admin"}); // Middleware to check if user is admin
						
						// Camera settings
						camera.get("/settings", [](Request& request) -> Response {
								return CameraController::getSettings(request);
						}).name("api.camera.settings.get");
						
						camera.post("/settings", [](Request& request) -> Response {
								return CameraController::updateSettings(request);
						}).name("api.camera.settings.update");
						
						// Camera capture
						camera.post("/capture", [](Request& request) -> Response {
								return CameraController::capture(request);
						}).name("api.camera.capture");
						
						// Camera status and control
						camera.get("/status", [](Request& request) -> Response {
								return CameraController::getStatus(request);
						}).name("api.camera.status");
						
						camera.post("/enable", [](Request& request) -> Response {
								return CameraController::enable(request);
						}).name("api.camera.enable");
						
						camera.post("/disable", [](Request& request) -> Response {
								return CameraController::disable(request);
						}).name("api.camera.disable");
				});

				// System routes
				api.group("/system", [&](Router& system) {
						system.middleware({"auth", "admin"}); // Require authentication
						
						// Get system statistics
						system.get("/stats", [](Request& request) -> Response {
								return SystemController::getStats(request);
						}).name("api.system.stats");
						
						// Get detailed memory information
						system.get("/memory", [](Request& request) -> Response {
								return SystemController::getMemoryInfo(request);
						}).name("api.system.memory");
						
						// Get network information
						system.get("/network", [](Request& request) -> Response {
								return SystemController::getNetworkInfo(request);
						}).name("api.system.network");
						
						// Get hostname information
						system.get("/hostname", [](Request& request) -> Response {
								return SystemController::getHostname(request);
						}).name("api.system.hostname.get");
						
						// Update hostname
						system.post("/hostname", [](Request& request) -> Response {
								return SystemController::updateHostname(request);
						}).name("api.system.hostname.update");
						
						// Get all configurations
						system.get("/configurations", [](Request& request) -> Response {
								return SystemController::getConfigurations(request);
						}).name("api.system.configs.get");
						
						// Update a configuration
						system.post("/configuration", [](Request& request) -> Response {
								return SystemController::updateConfiguration(request);
						}).name("api.system.configs.update");
						
						// System restart (admin only)
						system.post("/restart", [](Request& request) -> Response {
								return SystemController::restart(request);
						}).name("api.system.restart");
				});

				// Servo routes
				api.group("/servo", [&](Router& servo) {
						servo.middleware({"auth", "admin"}); // Require authentication and admin role
						
						// Servo management
						servo.get("/list", [](Request& request) -> Response {
								return ServoController::getServos(request);
						}).name("api.servo.list");
						
						servo.post("/add", [](Request& request) -> Response {
								return ServoController::addServo(request);
						}).name("api.servo.add");
						
						servo.delete_("/remove", [](Request& request) -> Response {
								return ServoController::removeServo(request);
						}).name("api.servo.remove");
						
						// Servo control
						servo.post("/angle", [](Request& request) -> Response {
								return ServoController::setAngle(request);
						}).name("api.servo.set_angle");
						
						servo.post("/angle/smooth", [](Request& request) -> Response {
								return ServoController::setAngleSmooth(request);
						}).name("api.servo.set_angle_smooth");
						
						servo.get("/angle", [](Request& request) -> Response {
								return ServoController::getAngle(request);
						}).name("api.servo.get_angle");
						
						// Servo status and control
						servo.get("/status", [](Request& request) -> Response {
								return ServoController::getStatus(request);
						}).name("api.servo.status");
						
						servo.post("/enable", [](Request& request) -> Response {
								return ServoController::enableServo(request);
						}).name("api.servo.enable");
						
						servo.post("/disable", [](Request& request) -> Response {
								return ServoController::disableServo(request);
						}).name("api.servo.disable");
						
						// Bulk operations
						servo.post("/enable-all", [](Request& request) -> Response {
								return ServoController::enableAllServos(request);
						}).name("api.servo.enable_all");
						
						servo.post("/disable-all", [](Request& request) -> Response {
								return ServoController::disableAllServos(request);
						}).name("api.servo.disable_all");
						
						servo.post("/set-all", [](Request& request) -> Response {
								return ServoController::setAllAngles(request);
						}).name("api.servo.set_all");
						
						servo.post("/sweep", [](Request& request) -> Response {
								return ServoController::sweepAllServos(request);
						}).name("api.servo.sweep");
						
						// Preset management
						servo.post("/preset/save", [](Request& request) -> Response {
								return ServoController::savePreset(request);
						}).name("api.servo.preset.save");
						
						servo.post("/preset/load", [](Request& request) -> Response {
								return ServoController::loadPreset(request);
						}).name("api.servo.preset.load");
						
						servo.get("/preset/list", [](Request& request) -> Response {
								return ServoController::getPresets(request);
						}).name("api.servo.preset.list");
						
						servo.delete_("/preset/delete", [](Request& request) -> Response {
								return ServoController::deletePreset(request);
						}).name("api.servo.preset.delete");
						
						// Quick setup endpoints
						servo.post("/setup/pantilt", [](Request& request) -> Response {
								return ServoController::setupPanTilt(request);
						}).name("api.servo.setup.pantilt");
						
						servo.post("/setup/arm", [](Request& request) -> Response {
								return ServoController::setupRoboticArm(request);
						}).name("api.servo.setup.arm");
						
						servo.post("/setup/array", [](Request& request) -> Response {
								return ServoController::setupMultiArray(request);
						}).name("api.servo.setup.array");
						
						// Database management endpoints
						servo.post("/config/save", [](Request& request) -> Response {
								return ServoController::saveConfigToDatabase(request);
						}).name("api.servo.config.save");
						
						servo.post("/config/load", [](Request& request) -> Response {
								return ServoController::loadConfigFromDatabase(request);
						}).name("api.servo.config.load");
						
						servo.get("/config/list", [](Request& request) -> Response {
								return ServoController::getSavedConfigs(request);
						}).name("api.servo.config.list");
						
						servo.delete_("/config/delete", [](Request& request) -> Response {
								return ServoController::deleteSavedConfig(request);
						}).name("api.servo.config.delete");
						
						servo.post("/config/save-all", [](Request& request) -> Response {
								return ServoController::saveAllConfigs(request);
						}).name("api.servo.config.save_all");
						
						servo.post("/config/load-all", [](Request& request) -> Response {
								return ServoController::loadAllConfigs(request);
						}).name("api.servo.config.load_all");
				});

				// Legacy restart endpoint (for backward compatibility)
				api.post("/restart", [](Request& request) -> Response {
						return SystemController::restart(request);
				});
		});
}

void registerWebSocketRoutes(Router* router) {
		// Camera WebSocket for frame streaming
		router->websocket("/ws/camera")
				.onConnect([](WebSocketRequest& request) {
						Serial.printf("[WebSocket] Camera client %u connected\n", request.clientId());
						
						// Send welcome message
						JsonDocument welcome;
						welcome["type"] = "welcome";
						welcome["message"] = "Connected to camera stream";
						
						String welcomeMsg;
						serializeJson(welcome, welcomeMsg);
						request.send(welcomeMsg);
				})
				.onDisconnect([](WebSocketRequest& request) {
						Serial.printf("[WebSocket] Camera client %u disconnected\n", request.clientId());
				})
				.onMessage([](WebSocketRequest& request, const String& message) {
						// Handle camera commands
						JsonDocument doc;
						DeserializationError error = deserializeJson(doc, message);
						
						if (error) {
								Serial.println("[WebSocket] Invalid JSON received");
								return;
						}
						
						String command = doc["command"].as<String>();
						
						if (command == "capture") {
								// Capture and send frame
								Camera& camera = Camera::getInstance();
								CameraFrame frame = camera.captureJPEG();
								
								if (frame.buf && frame.len > 0) {
										// Send binary frame data
										request.send(frame.buf, frame.len);
										camera.releaseFrame(frame);
								} else {
										// Send error message
										JsonDocument errorResponse;
										errorResponse["type"] = "error";
										errorResponse["message"] = "Failed to capture frame";
										
										String errorMsg;
										serializeJson(errorResponse, errorMsg);
										request.send(errorMsg);
								}
						}
						else if (command == "ping") {
								// Respond with pong
								JsonDocument pongResponse;
								pongResponse["type"] = "pong";
								pongResponse["timestamp"] = millis();
								
								String pongMsg;
								serializeJson(pongResponse, pongMsg);
								request.send(pongMsg);
						}
				});
}
