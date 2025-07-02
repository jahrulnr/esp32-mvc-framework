#include "routes.h"
#include <SPIFFS.h>

void registerWebRoutes(Router* router) {
		HomeController* homeController = new HomeController();
		AuthController* authController = new AuthController();
		
		// Home routes
		router->get("/", [homeController](Request& request) -> Response {
				return homeController->index(request);
		}).name("home");
		
		router->get("/about", [homeController](Request& request) -> Response {
				return homeController->about(request);
		}).name("about");
		
		router->get("/contact", [homeController](Request& request) -> Response {
				return homeController->contact(request);
		}).name("contact.show");
		
		router->post("/contact", [homeController](Request& request) -> Response {
				return homeController->contact(request);
		}).name("contact.store");
		
		// CSV Database Demo
		router->get("/csv-demo", [](Request& request) -> Response {
				if (SPIFFS.exists("/views/csv-demo.html")) {
						File file = SPIFFS.open("/views/csv-demo.html", "r");
						String html = file.readString();
						file.close();
						
						return Response(request.getServerRequest())
								.html(html);
				}
				
				return Response(request.getServerRequest())
						.status(404)
						.text("CSV Demo page not found");
		}).name("csv.demo");
		
		// User Management (Admin/System only)
		router->get("/users", [](Request& request) -> Response {
				if (SPIFFS.exists("/views/users.html")) {
						File file = SPIFFS.open("/views/users.html", "r");
						String html = file.readString();
						file.close();
						
						return Response(request.getServerRequest())
								.html(html);
				}
				
				return Response(request.getServerRequest())
						.status(404)
						.text("User management page not found");
		}).name("users.index");

		// Authentication routes
		router->get("/login", [authController](Request& request) -> Response {
				return authController->showLogin(request);
		}).name("login.show");
		
		router->post("/login", [authController](Request& request) -> Response {
				return authController->login(request);
		}).name("login");
		
		router->get("/register", [authController](Request& request) -> Response {
				return authController->showSignup(request);
		}).name("register.show");
		
		router->post("/register", [authController](Request& request) -> Response {
				return authController->signup(request);
		}).name("register");
		
		router->post("/logout", [authController](Request& request) -> Response {
				return authController->logout(request);
		}).name("logout");
		
		// Protected routes (client-side auth check)
		router->get("/dashboard", [authController](Request& request) -> Response {
				return authController->dashboard(request);
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
		ApiController* apiController = new ApiController();
		UserController* userController = new UserController();
		
		// API routes with middleware
		router->group("/api/v1", [&](Router& api) {
				api.middleware({"cors", "json", "ratelimit"});
				
				// Auth routes for user info (register first to avoid conflicts)
				api.group("/auth", [&](Router& auth) {
						AuthController* authController = new AuthController();
						
						auth.get("/user", [authController](Request& request) -> Response {
								return authController->getUserInfo(request);
						}).name("api.auth.user");
				});
				
				// Resource routes
				api.get("/items", [apiController](Request& request) -> Response {
						return apiController->index(request);
				}).name("api.items.index");
				
				api.get("/items/{id}", [apiController](Request& request) -> Response {
						return apiController->show(request);
				}).name("api.items.show");
				
				api.post("/items", [apiController](Request& request) -> Response {
						return apiController->store(request);
				}).name("api.items.store");
				
				api.put("/items/{id}", [apiController](Request& request) -> Response {
						return apiController->update(request);
				}).name("api.items.update");
				
				api.delete_("/items/{id}", [apiController](Request& request) -> Response {
						return apiController->destroy(request);
				}).name("api.items.destroy");
				
				// System routes
				api.get("/status", [apiController](Request& request) -> Response {
						return apiController->status(request);
				}).name("api.status");
				
				api.get("/health", [apiController](Request& request) -> Response {
						return apiController->health(request);
				}).name("api.health");
				
				api.get("/version", [apiController](Request& request) -> Response {
						return apiController->version(request);
				}).name("api.version");
				
				// Users route (CSV database demo)
				api.get("/users", [apiController](Request& request) -> Response {
						return apiController->users(request);
				}).name("api.users");
				
				// Admin User Management Routes (Role-based CRUD)
				api.group("/admin", [&](Router& admin) {
						admin.middleware("auth"); // Add auth middleware for admin routes
						
						// User CRUD operations
						admin.get("/users", [userController](Request& request) -> Response {
								return userController->apiIndex(request);
						}).name("api.admin.users.index");
						
						admin.get("/users/{id}", [userController](Request& request) -> Response {
								return userController->apiShow(request);
						}).name("api.admin.users.show");
						
						admin.post("/users", [userController](Request& request) -> Response {
								return userController->apiStore(request);
						}).name("api.admin.users.store");
						
						admin.put("/users/{id}", [userController](Request& request) -> Response {
								return userController->apiUpdate(request);
						}).name("api.admin.users.update");
						
						admin.delete_("/users/{id}", [userController](Request& request) -> Response {
								return userController->apiDestroy(request);
						}).name("api.admin.users.destroy");
				});
				
				// Protected API routes
				api.get("/dashboard", [](Request& request) -> Response {
						AuthController authController;
						return authController.dashboard(request);
				}).middleware("auth").name("api.dashboard");

				api.post("/restart", [](Request& request) -> Response {
						ESP.restart();
						return Response(request.getServerRequest())
								.status(204);
				});
		});
}
