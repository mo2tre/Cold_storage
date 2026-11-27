#include "controllers/FarmerController.h"

#include <drogon/drogon.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include "services/AuthService.h"
#include "services/FarmerService.h"
#include "utils/ApiResponse.h"

namespace controllers
{
namespace
{
using Callback = std::function<void(const drogon::HttpResponsePtr&)>;

bool parseId(const std::string& s, uint64_t& id)
{
    if (s.empty() || s.size() > 18 ||
        !std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c) != 0; }))
        return false;
    id = std::strtoull(s.c_str(), nullptr, 10);
    return id > 0;
}
}  // namespace

void registerFarmerRoutes()
{
    drogon::app().registerHandler(
        "/api/v1/farmers",
        [](const drogon::HttpRequestPtr& req, Callback&& callback) {
            const bool isPost = req->method() == drogon::Post;
            Callback cb = std::move(callback);
            services::AuthService::authorize(
                req, isPost ? "farmers.write" : "farmers.read",
                [req, cb, isPost](const services::AuthUser&) {
                    if (isPost)
                        services::FarmerService::create(req, cb);
                    else
                        services::FarmerService::list(req, cb);
                },
                cb);
        },
        {drogon::Get, drogon::Post});

    drogon::app().registerHandler(
        "/api/v1/farmers/{id}",
        [](const drogon::HttpRequestPtr& req, Callback&& callback, std::string id) {
            Callback cb = std::move(callback);
            uint64_t farmerId = 0;
            if (!parseId(id, farmerId))
            {
                cb(api::badRequest("Farmer id must be a positive integer"));
                return;
            }
            const bool isPut = req->method() == drogon::Put;
            services::AuthService::authorize(
                req, isPut ? "farmers.write" : "farmers.read",
                [req, cb, isPut, farmerId](const services::AuthUser&) {
                    if (isPut)
                        services::FarmerService::update(farmerId, req, cb);
                    else
                        services::FarmerService::getById(farmerId, cb);
                },
                cb);
        },
        {drogon::Get, drogon::Put});
}
}  // namespace controllers
