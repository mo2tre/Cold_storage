#include <drogon/drogon.h>

#include <string>

#include "controllers/FarmerController.h"
#include "controllers/HealthController.h"
#include "utils/ApiResponse.h"

int main(int argc, char* argv[])
{
    const std::string configPath = (argc > 1) ? argv[1] : "config/config.json";

    // Unknown routes return the standard error envelope instead of an HTML page.
    drogon::app().setCustom404Page(api::notFound("Endpoint not found"));

    // Register every controller module here.
    controllers::registerHealthRoutes();
    controllers::registerFarmerRoutes();

    drogon::app().loadConfigFile(configPath);
    drogon::app().run();
    return 0;
}
