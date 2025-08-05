#ifndef ROUTES_H
#define ROUTES_H

#include <MVCFramework.h>
#include "Controllers/HomeController.h"
#include "Controllers/ApiController.h"
#include "Controllers/AuthController.h"
#include "Controllers/UserController.h"

void registerWebRoutes(Router* router);
void registerApiRoutes(Router* router);

#endif
