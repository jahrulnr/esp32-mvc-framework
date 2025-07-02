#ifndef ROUTES_H
#define ROUTES_H

#include "Framework/Core/Application.h"
#include "Controllers/HomeController.h"
#include "Controllers/ApiController.h"
#include "Controllers/AuthController.h"

void registerWebRoutes(Router* router);
void registerApiRoutes(Router* router);

#endif
