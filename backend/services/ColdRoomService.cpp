#include "services/ColdRoomService.h"

#include "dto/ColdRoomDto.h"
#include "repositories/ColdRoomRepository.h"

namespace services
{
namespace
{
using repositories::ColdRoomRepository;
const char* kDuplicate = "A cold room with this code already exists";

Json::Value idsJson(const std::vector<uint64_t>& ids)
{
    Json::Value data;
    Json::Value arr(Json::arrayValue);
    for (auto v : ids)
        arr.append(static_cast<Json::UInt64>(v));
    data["product_ids"] = arr;
    return data;
}
}  // namespace

void ColdRoomService::list(const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    common::ListParams p;
    if (!common::parseListParams(req, cb, p))
        return;
    ColdRoomRepository::list(
        p.page, p.pageSize, p.search, p.activeFilter,
        [cb, p](Json::Value items, uint64_t total) {
            cb(api::ok(common::pageJson(items, p.page, p.pageSize, total)));
        },
        common::dbErrorHandler(cb, kDuplicate));
}

void ColdRoomService::getById(uint64_t id, ResponseCb cb)
{
    ColdRoomRepository::findById(
        id,
        [cb](std::optional<Json::Value> room) {
            if (!room)
                cb(api::notFound("Cold room not found"));
            else
                cb(api::ok(*room));
        },
        common::dbErrorHandler(cb, kDuplicate));
}

void ColdRoomService::create(const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    auto body = req->getJsonObject();
    if (!body)
    {
        cb(api::badRequest("Request body must be valid JSON"));
        return;
    }
    dto::ColdRoomInput input;
    Json::Value details = dto::parseColdRoomInput(*body, input);
    if (!details.empty())
    {
        cb(api::validationError("Invalid cold room data", details));
        return;
    }
    ColdRoomRepository::create(
        input, [cb](Json::Value room) { cb(api::created(room, "Cold room created")); },
        common::dbErrorHandler(cb, kDuplicate));
}

void ColdRoomService::patch(uint64_t id, const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    auto body = req->getJsonObject();
    if (!body || !body->isObject())
    {
        cb(api::badRequest("Request body must be a JSON object"));
        return;
    }
    const auto onDbError = common::dbErrorHandler(cb, kDuplicate);
    const Json::Value changes = *body;
    ColdRoomRepository::findById(
        id,
        [cb, changes, id, onDbError](std::optional<Json::Value> existing) {
            if (!existing)
            {
                cb(api::notFound("Cold room not found"));
                return;
            }
            Json::Value merged = *existing;
            for (const auto& key : changes.getMemberNames())
                merged[key] = changes[key];

            dto::ColdRoomInput input;
            Json::Value details = dto::parseColdRoomInput(merged, input);
            if (!details.empty())
            {
                cb(api::validationError("Invalid cold room data", details));
                return;
            }
            ColdRoomRepository::update(
                id, input,
                [cb](std::optional<Json::Value> room) {
                    if (!room)
                        cb(api::notFound("Cold room not found"));
                    else
                        cb(api::ok(*room, "Cold room updated"));
                },
                onDbError);
        },
        onDbError);
}

void ColdRoomService::getProducts(uint64_t id, ResponseCb cb)
{
    const auto onDbError = common::dbErrorHandler(cb, kDuplicate);
    ColdRoomRepository::findById(
        id,
        [cb, id, onDbError](std::optional<Json::Value> room) {
            if (!room)
            {
                cb(api::notFound("Cold room not found"));
                return;
            }
            ColdRoomRepository::listProductIds(
                id, [cb](std::vector<uint64_t> ids) { cb(api::ok(idsJson(ids))); }, onDbError);
        },
        onDbError);
}

void ColdRoomService::setProducts(uint64_t id, const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    auto body = req->getJsonObject();
    if (!body)
    {
        cb(api::badRequest("Request body must be valid JSON"));
        return;
    }
    std::vector<uint64_t> productIds;
    Json::Value details = dto::parseProductIds(*body, productIds);
    if (!details.empty())
    {
        cb(api::validationError("Invalid product list", details));
        return;
    }

    const auto onDbError = common::dbErrorHandler(cb, kDuplicate);
    ColdRoomRepository::findById(
        id,
        [cb, id, productIds, onDbError](std::optional<Json::Value> room) {
            if (!room)
            {
                cb(api::notFound("Cold room not found"));
                return;
            }
            ColdRoomRepository::setProductIds(
                id, productIds,
                [cb](std::vector<uint64_t> ids) { cb(api::ok(idsJson(ids), "Compatible products updated")); },
                [cb]() {
                    Json::Value d(Json::arrayValue);
                    Json::Value item;
                    item["field"] = "product_ids";
                    item["message"] = "contains an unknown product id";
                    d.append(item);
                    cb(api::validationError("Invalid product list", d));
                },
                onDbError);
        },
        onDbError);
}
}  // namespace services
