#include "dto/ProductDto.h"

#include "utils/Validation.h"

namespace dto
{
Json::Value parseProductInput(const Json::Value& body, ProductInput& out)
{
    Json::Value details(Json::arrayValue);
    if (!body.isObject())
    {
        validation::addError(details, "body", "must be a JSON object");
        return details;
    }
    out.code = validation::requiredString(body, "code", 30, details);
    out.name = validation::requiredString(body, "name", 150, details);
    validation::parseTemperatureRange(body, out.minTempC, out.maxTempC, details);
    validation::parseHumidityRange(body, out.minHumidityPct, out.maxHumidityPct, details);
    validation::optionalPositiveInt(body, "max_storage_days", 36500, out.maxStorageDays, details);

    if (body.isMember("is_active"))
    {
        if (!body["is_active"].isBool())
            validation::addError(details, "is_active", "must be true or false");
        else
            out.isActive = body["is_active"].asBool();
    }
    return details;
}
}  // namespace dto
