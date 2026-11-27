#include <drogon/drogon.h>

#include <iostream>
#include <string>

#include "controllers/AuthController.h"
#include "controllers/ColdRoomController.h"
#include "controllers/FarmerController.h"
#include "controllers/HealthController.h"
#include "controllers/ProductController.h"
#include "utils/ApiResponse.h"
#include "utils/Crypto.h"
#include "utils/PasswordPrompt.h"

namespace
{
// cold_storage_backend.exe --hash-password
// Prints a password hash to paste into the users table (used to create the first administrator).
int runHashPassword()
{
    const std::string p1 = utils::readPasswordHidden("New password: ");
    const std::string p2 = utils::readPasswordHidden("Repeat password: ");
    if (p1 != p2)
    {
        std::cerr << "Passwords do not match.\n";
        return 1;
    }
    if (p1.size() < 8)
    {
        std::cerr << "Password must be at least 8 characters.\n";
        return 1;
    }
    std::cout << utils::hashPassword(p1) << "\n";
    return 0;
}
}  // namespace

int main(int argc, char* argv[])
{
    if (argc > 1 && std::string(argv[1]) == "--hash-password")
        return runHashPassword();

    const std::string configPath = (argc > 1) ? argv[1] : "config/config.json";

    // Unknown routes return the standard error envelope instead of an HTML page.
    drogon::app().setCustom404Page(api::notFound("Endpoint not found"));

    // Register every controller module here.
    controllers::registerHealthRoutes();
    controllers::registerAuthRoutes();
    controllers::registerFarmerRoutes();
    controllers::registerProductRoutes();
    controllers::registerColdRoomRoutes();

    drogon::app().loadConfigFile(configPath);
    drogon::app().run();
    return 0;
}
