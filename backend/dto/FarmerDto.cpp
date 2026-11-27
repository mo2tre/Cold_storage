#include "dto/FarmerDto.h"

#include <algorithm>
#include <cctype>

namespace dto
{
namespace
{
std::string trim(const std::string& s)
{
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    auto b = std::find_if_not(s.begin(), s.end(), isSpace);
    auto e = std::find_if_not(s.rbegin(), s.rend(), isSpace).base();
    return (b < e) ? std::string(b, e) : std::string();
}

void addError(Json::Value& details, const std::string& field, const std::string& message)
{
    Json::Value d;
    d["field"] = field;
    d["message"] = message;
    details.append(d);
}

// Required string: present, a string, non-empty, <= maxLen.
std::string requiredString(const Json::Value& body, const char* field, size_t maxLen, Json::Value& details)
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

// Optional string: absent/null/empty -> NULL.
std::shared_ptr<std::string> optionalString(const Json::Value& body, const char* field, size_t maxLen,
                                            Json::Value& details)
{
    if (!body.isMember(field) || body[field].isNull())
        return nullptr;
    if (!body[field].isString())
    {
        addError(details, field, "must be a string");
        return nullptr;
    }
    std::string v = trim(body[field].asString());
    if (v.empty())
        return nullptr;
    if (v.size() > maxLen)
    {
        addError(details, field, "must be at most " + std::to_string(maxLen) + " characters");
        return nullptr;
    }
    return std::make_shared<std::string>(std::move(v));
}
}  // namespace

Json::Value parseFarmerInput(const Json::Value& body, FarmerInput& out)
{
    Json::Value details(Json::arrayValue);
    if (!body.isObject())
    {
        addError(details, "body", "must be a JSON object");
        return details;
    }

    out.code = requiredString(body, "code", 30, details);
    out.fullName = requiredString(body, "full_name", 150, details);
    out.phone = optionalString(body, "phone", 30, details);
    out.email = optionalString(body, "email", 150, details);
    out.address = optionalString(body, "address", 255, details);
    out.identifier = optionalString(body, "identifier", 50, details);
    out.notes = optionalString(body, "notes", 5000, details);

    if (out.email && out.email->find('@') == std::string::npos)
        addError(details, "email", "is not a valid email address");

    if (body.isMember("is_active"))
    {
        if (!body["is_active"].isBool())
            addError(details, "is_active", "must be true or false");
        else
            out.isActive = body["is_active"].asBool();
    }
    return details;
}
}  // namespace dto
