#include "controllers/HealthController.h"

#include <drogon/drogon.h>

#include "utils/ApiResponse.h"

namespace controllers
{
void registerHealthRoutes()
{
    drogon::app().registerHandler(
        "/api/v1/health",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            Json::Value data;
            data["status"] = "ok";
            data["service"] = "cold-storage-backend";
            callback(api::ok(data));
        },
        {drogon::Get});
}
}  // namespace controllers
