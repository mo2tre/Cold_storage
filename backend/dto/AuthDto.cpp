#include "dto/AuthDto.h"

#include <algorithm>
#include <cctype>

namespace dto
{
namespace
{
void addError(Json::Value& details, const std::string& field, const std::string& message)
{
    Json::Value d;
    d["field"] = field;
    d["message"] = message;
    details.append(d);
}

std::string trim(const std::string& s)
{
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    auto b = std::find_if_not(s.begin(), s.end(), isSpace);
    auto e = std::find_if_not(s.rbegin(), s.rend(), isSpace).base();
    return (b < e) ? std::string(b, e) : std::string();
}
}  // namespace

Json::Value parseLoginInput(const Json::Value& body, LoginInput& out)
{
    Json::Value details(Json::arrayValue);
    if (!body.isObject())
    {
        addError(details, "body", "must be a JSON object");
        return details;
    }

    if (!body.isMember("username") || !body["username"].isString() || trim(body["username"].asString()).empty())
        addError(details, "username", "is required");
    else
    {
        out.username = trim(body["username"].asString());
        if (out.username.size() > 50)
            addError(details, "username", "must be at most 50 characters");
    }

    // The password is never trimmed or logged.
    if (!body.isMember("password") || !body["password"].isString() || body["password"].asString().empty())
        addError(details, "password", "is required");
    else
    {
        out.password = body["password"].asString();
        if (out.password.size() > 200)
            addError(details, "password", "must be at most 200 characters");
    }
    return details;
}
}  // namespace dto
