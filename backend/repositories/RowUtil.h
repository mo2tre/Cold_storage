#pragma once

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include <cstdlib>
#include <string>

namespace repositories
{
// "2026-11-26 21:00:00.123" (stored in UTC) -> "2026-11-26T21:00:00.123Z"
inline std::string isoUtc(std::string s)
{
    if (s.size() > 10 && s[10] == ' ')
        s[10] = 'T';
    return s + "Z";
}

inline Json::Value decimalOrNull(const drogon::orm::Row& row, const char* column)
{
    if (row[column].isNull())
        return Json::Value(Json::nullValue);
    return Json::Value(std::strtod(row[column].as<std::string>().c_str(), nullptr));
}

inline Json::Value uintOrNull(const drogon::orm::Row& row, const char* column)
{
    if (row[column].isNull())
        return Json::Value(Json::nullValue);
    return Json::Value(static_cast<Json::UInt64>(row[column].as<uint64_t>()));
}
}  // namespace repositories
