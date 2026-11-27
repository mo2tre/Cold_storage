#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

#include <cstdint>
#include <functional>

namespace services
{
using ResponseCb = std::function<void(const drogon::HttpResponsePtr&)>;

// Business rules and validation for farmers. Responds through the standard JSON envelope.
class FarmerService
{
  public:
    static void list(const drogon::HttpRequestPtr& req, ResponseCb cb);
    static void getById(uint64_t id, ResponseCb cb);
    static void create(const drogon::HttpRequestPtr& req, ResponseCb cb);
    static void update(uint64_t id, const drogon::HttpRequestPtr& req, ResponseCb cb);
};
}  // namespace services
