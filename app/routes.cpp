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
        
        if (SPIFFS.exists(path)) {
            File assetFile = SPIFFS.open(path, "r");
            String content = assetFile.readString();
            assetFile.close();
            
            // Determine content type based on file extension
            String contentType = "text/plain";
            if (file.endsWith(".css")) {
                contentType = "text/css";
            } else if (file.endsWith(".js")) {
                contentType = "application/javascript";
            } else if (file.endsWith(".ico")) {
                contentType = "image/x-icon";
            } else if (file.endsWith(".png")) {
                contentType = "image/png";
            } else if (file.endsWith(".jpg") || file.endsWith(".jpeg")) {
                contentType = "image/jpeg";
            }
            
            return Response(request.getServerRequest())
                .contentType(contentType)
                .content(content);
        }
        
        return Response(request.getServerRequest())
            .status(404)
            .text("Asset not found");
    }).name("assets");


    router->get("/favicon.ico", [](Request& request) -> Response {
			String favicon = "/favicon.ico";
			if (SPIFFS.exists(favicon)) {
				File assetFile = SPIFFS.open(favicon, "r");
				String content = assetFile.readString();
				assetFile.close();

				return Response(request.getServerRequest())
                .contentType("image/x-icon")
                .content(content);
			}
        
			return Response(request.getServerRequest())
					.status(404)
					.text("Favicon not found");
		});
}

void registerApiRoutes(Router* router) {
    ApiController* apiController = new ApiController();
    
    // API routes with middleware
    router->group("/api/v1", [&](Router& api) {
        api.middleware({"cors", "json", "ratelimit"});
        
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
        
        // Protected API routes
        api.get("/dashboard", [](Request& request) -> Response {
            AuthController authController;
            return authController.dashboard(request);
        }).middleware("auth").name("api.dashboard");
    });
}
