#include "controllers/FarmerController.h"

#include <drogon/drogon.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>

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
            if (req->method() == drogon::Post)
                services::FarmerService::create(req, std::move(callback));
            else
                services::FarmerService::list(req, std::move(callback));
        },
        {drogon::Get, drogon::Post});

    drogon::app().registerHandler(
        "/api/v1/farmers/{id}",
        [](const drogon::HttpRequestPtr& req, Callback&& callback, std::string id) {
            uint64_t farmerId = 0;
            if (!parseId(id, farmerId))
            {
                callback(api::badRequest("Farmer id must be a positive integer"));
                return;
            }
            if (req->method() == drogon::Put)
                services::FarmerService::update(farmerId, req, std::move(callback));
            else
                services::FarmerService::getById(farmerId, std::move(callback));
        },
        {drogon::Get, drogon::Put});
}
}  // namespace controllers
