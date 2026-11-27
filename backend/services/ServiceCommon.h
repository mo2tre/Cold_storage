#pragma once
// Helpers shared by the services: list query parameters and database error mapping.

#include <drogon/drogon.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <optional>
#include <string>

#include "repositories/RepoError.h"
#include "utils/ApiResponse.h"

namespace services
{
using ResponseCb = std::function<void(const drogon::HttpResponsePtr&)>;

namespace common
{
struct ListParams
{
    uint64_t page = 1;
    uint64_t pageSize = 20;
    int activeFilter = -1;  // -1 all, 0 inactive, 1 active
    std::string search;
};

inline std::optional<uint64_t> parseUInt(const std::string& s, uint64_t fallback)
{
    if (s.empty())
        return fallback;
    if (s.size() > 18 || !std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c) != 0; }))
        return std::nullopt;
    return std::strtoull(s.c_str(), nullptr, 10);
}

// Parses page, page_size, is_active and q. Answers 400 and returns false when invalid.
inline bool parseListParams(const drogon::HttpRequestPtr& req, const ResponseCb& cb, ListParams& out)
{
    auto page = parseUInt(req->getParameter("page"), 1);
    auto pageSize = parseUInt(req->getParameter("page_size"), 20);
    if (!page || *page < 1 || !pageSize || *pageSize < 1 || *pageSize > 100)
    {
        cb(api::badRequest("page must be >= 1 and page_size must be between 1 and 100"));
        return false;
    }
    out.page = *page;
    out.pageSize = *pageSize;

    const std::string active = req->getParameter("is_active");
    if (active == "true" || active == "1")
        out.activeFilter = 1;
    else if (active == "false" || active == "0")
        out.activeFilter = 0;
    else if (!active.empty())
    {
        cb(api::badRequest("is_active must be true or false"));
        return false;
    }
    out.search = req->getParameter("q");
    return true;
}

inline Json::Value pageJson(const Json::Value& items, uint64_t page, uint64_t pageSize, uint64_t total)
{
    Json::Value data;
    data["items"] = items;
    data["page"] = static_cast<Json::UInt64>(page);
    data["page_size"] = static_cast<Json::UInt64>(pageSize);
    data["total"] = static_cast<Json::UInt64>(total);
    return data;
}

// Internal details are logged, never sent to the client.
inline repositories::ErrorCb dbErrorHandler(ResponseCb cb, const std::string& duplicateMessage)
{
    return [cb, duplicateMessage](const repositories::RepoError& err) {
        if (err.duplicate)
        {
            cb(api::conflict(duplicateMessage));
            return;
        }
        LOG_ERROR << "Database error: " << err.message;
        cb(api::internalError());
    };
}

// Valid id from a path segment (positive integer).
inline bool parseId(const std::string& s, uint64_t& id)
{
    if (s.empty() || s.size() > 18 ||
        !std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c) != 0; }))
        return false;
    id = std::strtoull(s.c_str(), nullptr, 10);
    return id > 0;
}
}  // namespace common
}  // namespace services
