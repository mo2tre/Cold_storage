#pragma once

#include <json/json.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dto
{
struct ColdRoomInput
{
    std::string code;
    std::string name;
    double capacityKg = 0;
    double minTempC = 0;
    double maxTempC = 0;
    std::optional<double> minHumidityPct;
    std::optional<double> maxHumidityPct;
    bool isActive = true;
};

Json::Value parseColdRoomInput(const Json::Value& body, ColdRoomInput& out);

// Body of PUT /cold-rooms/{id}/products: { "product_ids": [1, 2, 3] }  (duplicates are removed)
Json::Value parseProductIds(const Json::Value& body, std::vector<uint64_t>& out);
}  // namespace dto
