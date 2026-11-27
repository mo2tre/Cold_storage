#include "controllers/ProductController.h"

#include <drogon/drogon.h>

#include "services/AuthService.h"
#include "services/ProductService.h"

namespace controllers
{
namespace
{
using Callback = std::function<void(const drogon::HttpResponsePtr&)>;
}

void registerProductRoutes()
{
    drogon::app().registerHandler(
        "/api/v1/products",
        [](const drogon::HttpRequestPtr& req, Callback&& callback) {
            const bool isPost = req->method() == drogon::Post;
            Callback cb = std::move(callback);
            services::AuthService::authorize(
                req, isPost ? "products.write" : "products.read",
                [req, cb, isPost](const services::AuthUser&) {
                    if (isPost)
                        services::ProductService::create(req, cb);
                    else
                        services::ProductService::list(req, cb);
                },
                cb);
        },
        {drogon::Get, drogon::Post});

    drogon::app().registerHandler(
        "/api/v1/products/{id}",
        [](const drogon::HttpRequestPtr& req, Callback&& callback, std::string id) {
            Callback cb = std::move(callback);
            uint64_t productId = 0;
            if (!services::common::parseId(id, productId))
            {
                cb(api::badRequest("Product id must be a positive integer"));
                return;
            }
            const bool isPatch = req->method() == drogon::Patch;
            services::AuthService::authorize(
                req, isPatch ? "products.write" : "products.read",
                [req, cb, isPatch, productId](const services::AuthUser&) {
                    if (isPatch)
                        services::ProductService::patch(productId, req, cb);
                    else
                        services::ProductService::getById(productId, cb);
                },
                cb);
        },
        {drogon::Get, drogon::Patch});
}
}  // namespace controllers
