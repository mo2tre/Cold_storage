#pragma once

#include <json/json.h>

#include <cstdint>
#include <optional>
#include <string>

namespace dto
{
struct ProductInput
{
    std::string code;
    std::string name;
    double minTempC = 0;
    double maxTempC = 0;
    std::optional<double> minHumidityPct;
    std::optional<double> maxHumidityPct;
    std::optional<uint32_t> maxStorageDays;
    bool isActive = true;
};

// Returns an array of {field, message}; empty means valid.
Json::Value parseProductInput(const Json::Value& body, ProductInput& out);
}  // namespace dto
