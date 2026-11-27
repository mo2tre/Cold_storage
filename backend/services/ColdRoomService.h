#pragma once

#include <drogon/HttpRequest.h>

#include <cstdint>

#include "services/ServiceCommon.h"

namespace services
{
class ColdRoomService
{
  public:
    static void list(const drogon::HttpRequestPtr& req, ResponseCb cb);
    static void getById(uint64_t id, ResponseCb cb);
    static void create(const drogon::HttpRequestPtr& req, ResponseCb cb);
    static void patch(uint64_t id, const drogon::HttpRequestPtr& req, ResponseCb cb);

    // Compatible products of a room
    static void getProducts(uint64_t id, ResponseCb cb);
    static void setProducts(uint64_t id, const drogon::HttpRequestPtr& req, ResponseCb cb);
};
}  // namespace services
