#include "services/ProductService.h"

#include "dto/ProductDto.h"
#include "repositories/ProductRepository.h"

namespace services
{
namespace
{
using repositories::ProductRepository;
const char* kDuplicate = "A product with this code already exists";
}  // namespace

void ProductService::list(const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    common::ListParams p;
    if (!common::parseListParams(req, cb, p))
        return;
    ProductRepository::list(
        p.page, p.pageSize, p.search, p.activeFilter,
        [cb, p](Json::Value items, uint64_t total) {
            cb(api::ok(common::pageJson(items, p.page, p.pageSize, total)));
        },
        common::dbErrorHandler(cb, kDuplicate));
}

void ProductService::getById(uint64_t id, ResponseCb cb)
{
    ProductRepository::findById(
        id,
        [cb](std::optional<Json::Value> product) {
            if (!product)
                cb(api::notFound("Product not found"));
            else
                cb(api::ok(*product));
        },
        common::dbErrorHandler(cb, kDuplicate));
}

void ProductService::create(const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    auto body = req->getJsonObject();
    if (!body)
    {
        cb(api::badRequest("Request body must be valid JSON"));
        return;
    }
    dto::ProductInput input;
    Json::Value details = dto::parseProductInput(*body, input);
    if (!details.empty())
    {
        cb(api::validationError("Invalid product data", details));
        return;
    }
    ProductRepository::create(
        input, [cb](Json::Value product) { cb(api::created(product, "Product created")); },
        common::dbErrorHandler(cb, kDuplicate));
}

void ProductService::patch(uint64_t id, const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    auto body = req->getJsonObject();
    if (!body || !body->isObject())
    {
        cb(api::badRequest("Request body must be a JSON object"));
        return;
    }
    const auto onDbError = common::dbErrorHandler(cb, kDuplicate);
    const Json::Value changes = *body;
    ProductRepository::findById(
        id,
        [cb, changes, id, onDbError](std::optional<Json::Value> existing) {
            if (!existing)
            {
                cb(api::notFound("Product not found"));
                return;
            }
            // Overlay the changes on the current values, then validate the complete product.
            Json::Value merged = *existing;
            for (const auto& key : changes.getMemberNames())
                merged[key] = changes[key];

            dto::ProductInput input;
            Json::Value details = dto::parseProductInput(merged, input);
            if (!details.empty())
            {
                cb(api::validationError("Invalid product data", details));
                return;
            }
            ProductRepository::update(
                id, input,
                [cb](std::optional<Json::Value> product) {
                    if (!product)
                        cb(api::notFound("Product not found"));
                    else
                        cb(api::ok(*product, "Product updated"));
                },
                onDbError);
        },
        onDbError);
}
}  // namespace services
