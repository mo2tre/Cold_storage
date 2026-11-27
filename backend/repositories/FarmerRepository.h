#pragma once

#include <json/json.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include "dto/FarmerDto.h"
#include "repositories/RepoError.h"

namespace repositories
{
// All SQL is parameterized. Every call is asynchronous.
class FarmerRepository
{
  public:
    // activeFilter: -1 = all, 0 = inactive only, 1 = active only
    static void list(uint64_t page, uint64_t pageSize, const std::string& search, int activeFilter,
                     std::function<void(Json::Value items, uint64_t total)> onOk, ErrorCb onErr);

    static void findById(uint64_t id, std::function<void(std::optional<Json::Value>)> onOk, ErrorCb onErr);

    static void create(const dto::FarmerInput& in, std::function<void(Json::Value)> onOk, ErrorCb onErr);

    // Replaces all editable fields. onOk receives nullopt when the id does not exist.
    static void update(uint64_t id, const dto::FarmerInput& in,
                       std::function<void(std::optional<Json::Value>)> onOk, ErrorCb onErr);
};
}  // namespace repositories
