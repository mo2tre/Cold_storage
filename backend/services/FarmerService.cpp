#include "services/FarmerService.h"

#include <drogon/drogon.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <memory>
#include <optional>

#include "dto/FarmerDto.h"
#include "repositories/FarmerRepository.h"
#include "utils/ApiResponse.h"

namespace services
{
namespace
{
using repositories::FarmerRepository;
using repositories::RepoError;

constexpr uint64_t kDefaultPageSize = 20;
constexpr uint64_t kMaxPageSize = 100;

// Unsigned integer query parameter with a default; nullopt when present but invalid.
std::optional<uint64_t> parseUInt(const std::string& s, uint64_t fallback)
{
    if (s.empty())
        return fallback;
    if (s.size() > 18 || !std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c) != 0; }))
        return std::nullopt;
    return std::strtoull(s.c_str(), nullptr, 10);
}

// Internal details are logged, never returned to the client.
repositories::ErrorCb dbErrorHandler(ResponseCb cb)
{
    return [cb](const RepoError& err) {
        if (err.duplicate)
        {
            cb(api::conflict("A farmer with this code already exists"));
            return;
        }
        LOG_ERROR << "Database error: " << err.message;
        cb(api::internalError());
    };
}
}  // namespace

void FarmerService::list(const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    auto page = parseUInt(req->getParameter("page"), 1);
    auto pageSize = parseUInt(req->getParameter("page_size"), kDefaultPageSize);
    if (!page || *page < 1 || !pageSize || *pageSize < 1 || *pageSize > kMaxPageSize)
    {
        cb(api::badRequest("page must be >= 1 and page_size must be between 1 and " +
                           std::to_string(kMaxPageSize)));
        return;
    }

    int activeFilter = -1;
    const std::string active = req->getParameter("is_active");
    if (active == "true" || active == "1")
        activeFilter = 1;
    else if (active == "false" || active == "0")
        activeFilter = 0;
    else if (!active.empty())
    {
        cb(api::badRequest("is_active must be true or false"));
        return;
    }

    const uint64_t p = *page;
    const uint64_t ps = *pageSize;
    FarmerRepository::list(
        p, ps, req->getParameter("q"), activeFilter,
        [cb, p, ps](Json::Value items, uint64_t total) {
            Json::Value data;
            data["items"] = items;
            data["page"] = static_cast<Json::UInt64>(p);
            data["page_size"] = static_cast<Json::UInt64>(ps);
            data["total"] = static_cast<Json::UInt64>(total);
            cb(api::ok(data));
        },
        dbErrorHandler(cb));
}

void FarmerService::getById(uint64_t id, ResponseCb cb)
{
    FarmerRepository::findById(
        id,
        [cb](std::optional<Json::Value> farmer) {
            if (!farmer)
                cb(api::notFound("Farmer not found"));
            else
                cb(api::ok(*farmer));
        },
        dbErrorHandler(cb));
}

void FarmerService::create(const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    auto body = req->getJsonObject();
    if (!body)
    {
        cb(api::badRequest("Request body must be valid JSON"));
        return;
    }

    dto::FarmerInput input;
    Json::Value details = dto::parseFarmerInput(*body, input);
    if (!details.empty())
    {
        cb(api::validationError("Invalid farmer data", details));
        return;
    }

    FarmerRepository::create(
        input, [cb](Json::Value farmer) { cb(api::created(farmer, "Farmer created")); }, dbErrorHandler(cb));
}

void FarmerService::update(uint64_t id, const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    auto body = req->getJsonObject();
    if (!body)
    {
        cb(api::badRequest("Request body must be valid JSON"));
        return;
    }

    dto::FarmerInput input;
    Json::Value details = dto::parseFarmerInput(*body, input);
    if (!details.empty())
    {
        cb(api::validationError("Invalid farmer data", details));
        return;
    }

    FarmerRepository::update(
        id, input,
        [cb](std::optional<Json::Value> farmer) {
            if (!farmer)
                cb(api::notFound("Farmer not found"));
            else
                cb(api::ok(*farmer, "Farmer updated"));
        },
        dbErrorHandler(cb));
}
}  // namespace services
