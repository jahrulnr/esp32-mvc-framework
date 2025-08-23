#ifndef ROUTES_H
#define ROUTES_H

#include <MVCFramework.h>
#include "../lib/MVCFramework/src/Routing/Router.h"
#include "../Controllers/AuthController.h"

void registerWebRoutes(Router* router);
void registerApiRoutes(Router* router);
void registerWebSocketRoutes(Router* router);

#endif
