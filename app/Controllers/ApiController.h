#ifndef API_CONTROLLER_H
#define API_CONTROLLER_H

#include "../Framework/Http/Controller.h"

class ApiController : public ResourceController {
public:
    Response index(Request& request) override;
    Response show(Request& request) override;
    Response store(Request& request) override;
    Response update(Request& request) override;
    Response destroy(Request& request) override;
    
    // Additional API methods
    Response status(Request& request);
    Response health(Request& request);
    Response version(Request& request);
    Response users(Request& request);
};

#endif
