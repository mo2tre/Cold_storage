#pragma once
// Small JSON validation helpers shared by the DTO parsers.

#include <json/json.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>

namespace validation
{
inline std::string trim(const std::string& s)
{
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    auto b = std::find_if_not(s.begin(), s.end(), isSpace);
    auto e = std::find_if_not(s.rbegin(), s.rend(), isSpace).base();
    return (b < e) ? std::string(b, e) : std::string();
}

inline void addError(Json::Value& details, const std::string& field, const std::string& message)
{
    Json::Value d;
    d["field"] = field;
    d["message"] = message;
    details.append(d);
}

inline std::string requiredString(const Json::Value& body, const char* field, size_t maxLen, Json::Value& details)
{
    if (!body.isMember(field) || body[field].isNull())
    {
        addError(details, field, "is required");
        return {};
    }
    if (!body[field].isString())
    {
        addError(details, field, "must be a string");
        return {};
    }
    std::string v = trim(body[field].asString());
    if (v.empty())
    {
        addError(details, field, "must not be empty");
        return {};
    }
    if (v.size() > maxLen)
    {
        addError(details, field, "must be at most " + std::to_string(maxLen) + " characters");
        return {};
    }
    return v;
}

inline bool requiredNumber(const Json::Value& body, const char* field, double lo, double hi, double& out,
                           Json::Value& details)
{
    if (!body.isMember(field) || body[field].isNull())
    {
        addError(details, field, "is required");
        return false;
    }
    if (!body[field].isNumeric())
    {
        addError(details, field, "must be a number");
        return false;
    }
    out = body[field].asDouble();
    if (out < lo || out > hi)
    {
        addError(details, field, "must be between " + std::to_string(static_cast<long long>(lo)) + " and " +
                                     std::to_string(static_cast<long long>(hi)));
        return false;
    }
    return true;
}

// Absent or null -> nullopt (valid). Returns false when present but invalid.
inline bool optionalNumber(const Json::Value& body, const char* field, double lo, double hi,
                           std::optional<double>& out, Json::Value& details)
{
    out.reset();
    if (!body.isMember(field) || body[field].isNull())
        return true;
    if (!body[field].isNumeric())
    {
        addError(details, field, "must be a number");
        return false;
    }
    double v = body[field].asDouble();
    if (v < lo || v > hi)
    {
        addError(details, field, "must be between " + std::to_string(static_cast<long long>(lo)) + " and " +
                                     std::to_string(static_cast<long long>(hi)));
        return false;
    }
    out = v;
    return true;
}

inline bool optionalPositiveInt(const Json::Value& body, const char* field, int64_t hi, std::optional<uint32_t>& out,
                                Json::Value& details)
{
    out.reset();
    if (!body.isMember(field) || body[field].isNull())
        return true;
    if (!body[field].isIntegral())
    {
        addError(details, field, "must be a whole number");
        return false;
    }
    int64_t v = body[field].asInt64();
    if (v < 1 || v > hi)
    {
        addError(details, field, "must be between 1 and " + std::to_string(hi));
        return false;
    }
    out = static_cast<uint32_t>(v);
    return true;
}

// Temperatures in degrees Celsius: both required, min <= max.
inline void parseTemperatureRange(const Json::Value& body, double& minT, double& maxT, Json::Value& details)
{
    const bool okMin = requiredNumber(body, "min_temp_c", -100, 100, minT, details);
    const bool okMax = requiredNumber(body, "max_temp_c", -100, 100, maxT, details);
    if (okMin && okMax && minT > maxT)
        addError(details, "min_temp_c", "must not be greater than max_temp_c");
}

// Humidity %: optional, but both or neither, 0..100, min <= max.
inline void parseHumidityRange(const Json::Value& body, std::optional<double>& minH, std::optional<double>& maxH,
                               Json::Value& details)
{
    const bool okMin = optionalNumber(body, "min_humidity_pct", 0, 100, minH, details);
    const bool okMax = optionalNumber(body, "max_humidity_pct", 0, 100, maxH, details);
    if (okMin && okMax)
    {
        if (minH.has_value() != maxH.has_value())
            addError(details, "min_humidity_pct", "min_humidity_pct and max_humidity_pct must be given together");
        else if (minH && maxH && *minH > *maxH)
            addError(details, "min_humidity_pct", "must not be greater than max_humidity_pct");
    }
}

inline std::string formatDecimal(double v, int decimals)
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals, v);
    return buf;
}
}  // namespace validation
