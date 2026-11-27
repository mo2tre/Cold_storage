#pragma once

#include <json/json.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "dto/ColdRoomDto.h"
#include "repositories/RepoError.h"

namespace repositories
{
class ColdRoomRepository
{
  public:
    static void list(uint64_t page, uint64_t pageSize, const std::string& search, int activeFilter,
                     std::function<void(Json::Value items, uint64_t total)> onOk, ErrorCb onErr);
    static void findById(uint64_t id, std::function<void(std::optional<Json::Value>)> onOk, ErrorCb onErr);
    static void create(const dto::ColdRoomInput& in, std::function<void(Json::Value)> onOk, ErrorCb onErr);
    static void update(uint64_t id, const dto::ColdRoomInput& in, std::function<void(std::optional<Json::Value>)> onOk,
                       ErrorCb onErr);

    // Compatibility (room_products)
    static void listProductIds(uint64_t roomId, std::function<void(std::vector<uint64_t>)> onOk, ErrorCb onErr);

    // Makes the room's compatible products exactly `productIds`. onUnknown is called (and nothing changes)
    // when one of the ids is not an existing product.
    static void setProductIds(uint64_t roomId, const std::vector<uint64_t>& productIds,
                              std::function<void(std::vector<uint64_t>)> onOk, std::function<void()> onUnknown,
                              ErrorCb onErr);
};
}  // namespace repositories
