#include "controllers/AuthController.h"

#include <drogon/drogon.h>

#include "services/AuthService.h"

namespace controllers
{
namespace
{
using Callback = std::function<void(const drogon::HttpResponsePtr&)>;
}

void registerAuthRoutes()
{
    drogon::app().registerHandler(
        "/api/v1/auth/login",
        [](const drogon::HttpRequestPtr& req, Callback&& callback) {
            services::AuthService::login(req, std::move(callback));
        },
        {drogon::Post});

    drogon::app().registerHandler(
        "/api/v1/auth/logout",
        [](const drogon::HttpRequestPtr& req, Callback&& callback) {
            services::AuthService::logout(req, std::move(callback));
        },
        {drogon::Post});

    drogon::app().registerHandler(
        "/api/v1/auth/me",
        [](const drogon::HttpRequestPtr& req, Callback&& callback) {
            services::AuthService::me(req, std::move(callback));
        },
        {drogon::Get});
}
}  // namespace controllers
