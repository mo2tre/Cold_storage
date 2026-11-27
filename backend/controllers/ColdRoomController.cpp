#include "controllers/ColdRoomController.h"

#include <drogon/drogon.h>

#include "services/AuthService.h"
#include "services/ColdRoomService.h"

namespace controllers
{
namespace
{
using Callback = std::function<void(const drogon::HttpResponsePtr&)>;
}

void registerColdRoomRoutes()
{
    drogon::app().registerHandler(
        "/api/v1/cold-rooms",
        [](const drogon::HttpRequestPtr& req, Callback&& callback) {
            const bool isPost = req->method() == drogon::Post;
            Callback cb = std::move(callback);
            services::AuthService::authorize(
                req, isPost ? "rooms.write" : "rooms.read",
                [req, cb, isPost](const services::AuthUser&) {
                    if (isPost)
                        services::ColdRoomService::create(req, cb);
                    else
                        services::ColdRoomService::list(req, cb);
                },
                cb);
        },
        {drogon::Get, drogon::Post});

    drogon::app().registerHandler(
        "/api/v1/cold-rooms/{id}",
        [](const drogon::HttpRequestPtr& req, Callback&& callback, std::string id) {
            Callback cb = std::move(callback);
            uint64_t roomId = 0;
            if (!services::common::parseId(id, roomId))
            {
                cb(api::badRequest("Cold room id must be a positive integer"));
                return;
            }
            const bool isPatch = req->method() == drogon::Patch;
            services::AuthService::authorize(
                req, isPatch ? "rooms.write" : "rooms.read",
                [req, cb, isPatch, roomId](const services::AuthUser&) {
                    if (isPatch)
                        services::ColdRoomService::patch(roomId, req, cb);
                    else
                        services::ColdRoomService::getById(roomId, cb);
                },
                cb);
        },
        {drogon::Get, drogon::Patch});

    drogon::app().registerHandler(
        "/api/v1/cold-rooms/{id}/products",
        [](const drogon::HttpRequestPtr& req, Callback&& callback, std::string id) {
            Callback cb = std::move(callback);
            uint64_t roomId = 0;
            if (!services::common::parseId(id, roomId))
            {
                cb(api::badRequest("Cold room id must be a positive integer"));
                return;
            }
            const bool isPut = req->method() == drogon::Put;
            services::AuthService::authorize(
                req, isPut ? "rooms.write" : "rooms.read",
                [req, cb, isPut, roomId](const services::AuthUser&) {
                    if (isPut)
                        services::ColdRoomService::setProducts(roomId, req, cb);
                    else
                        services::ColdRoomService::getProducts(roomId, cb);
                },
                cb);
        },
        {drogon::Get, drogon::Put});
}
}  // namespace controllers
