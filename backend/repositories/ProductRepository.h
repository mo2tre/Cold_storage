#pragma once

#include <json/json.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include "dto/ProductDto.h"
#include "repositories/RepoError.h"

namespace repositories
{
class ProductRepository
{
  public:
    static void list(uint64_t page, uint64_t pageSize, const std::string& search, int activeFilter,
                     std::function<void(Json::Value items, uint64_t total)> onOk, ErrorCb onErr);
    static void findById(uint64_t id, std::function<void(std::optional<Json::Value>)> onOk, ErrorCb onErr);
    static void create(const dto::ProductInput& in, std::function<void(Json::Value)> onOk, ErrorCb onErr);
    static void update(uint64_t id, const dto::ProductInput& in, std::function<void(std::optional<Json::Value>)> onOk,
                       ErrorCb onErr);
};
}  // namespace repositories
