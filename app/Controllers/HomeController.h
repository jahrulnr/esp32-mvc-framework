#ifndef HOME_CONTROLLER_H
#define HOME_CONTROLLER_H

#include <MVCFramework.h>

class HomeController : public Controller {
public:
    Response index(Request& request);
    Response about(Request& request);
    Response contact(Request& request);
};

#endif
