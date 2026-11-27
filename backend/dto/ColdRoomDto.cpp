#include "dto/ColdRoomDto.h"

#include <algorithm>

#include "utils/Validation.h"

namespace dto
{
Json::Value parseColdRoomInput(const Json::Value& body, ColdRoomInput& out)
{
    Json::Value details(Json::arrayValue);
    if (!body.isObject())
    {
        validation::addError(details, "body", "must be a JSON object");
        return details;
    }
    out.code = validation::requiredString(body, "code", 30, details);
    out.name = validation::requiredString(body, "name", 150, details);
    if (validation::requiredNumber(body, "capacity_kg", 0, 999999999.999, out.capacityKg, details) &&
        out.capacityKg <= 0)
        validation::addError(details, "capacity_kg", "must be greater than 0");
    validation::parseTemperatureRange(body, out.minTempC, out.maxTempC, details);
    validation::parseHumidityRange(body, out.minHumidityPct, out.maxHumidityPct, details);

    if (body.isMember("is_active"))
    {
        if (!body["is_active"].isBool())
            validation::addError(details, "is_active", "must be true or false");
        else
            out.isActive = body["is_active"].asBool();
    }
    return details;
}

Json::Value parseProductIds(const Json::Value& body, std::vector<uint64_t>& out)
{
    Json::Value details(Json::arrayValue);
    out.clear();
    if (!body.isObject() || !body.isMember("product_ids") || !body["product_ids"].isArray())
    {
        validation::addError(details, "product_ids", "must be an array of product ids");
        return details;
    }
    for (const auto& v : body["product_ids"])
    {
        if (!v.isIntegral() || v.asInt64() < 1)
        {
            validation::addError(details, "product_ids", "must contain only positive integers");
            return details;
        }
        out.push_back(static_cast<uint64_t>(v.asInt64()));
    }
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    if (out.size() > 1000)
        validation::addError(details, "product_ids", "too many ids");
    return details;
}
}  // namespace dto
