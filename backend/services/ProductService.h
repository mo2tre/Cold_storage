#pragma once

#include <drogon/HttpRequest.h>

#include <cstdint>

#include "services/ServiceCommon.h"

namespace services
{
class ProductService
{
  public:
    static void list(const drogon::HttpRequestPtr& req, ResponseCb cb);
    static void getById(uint64_t id, ResponseCb cb);
    static void create(const drogon::HttpRequestPtr& req, ResponseCb cb);
    // Partial update: only the fields present in the body change.
    static void patch(uint64_t id, const drogon::HttpRequestPtr& req, ResponseCb cb);
};
}  // namespace services
