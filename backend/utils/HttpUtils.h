#pragma once

#include <drogon/HttpRequest.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace utils
{
// Returns the token from "Authorization: Bearer <token>", or "" when absent/malformed.
inline std::string bearerToken(const drogon::HttpRequestPtr& req)
{
    const std::string header = req->getHeader("Authorization");
    const std::string prefix = "bearer ";
    if (header.size() <= prefix.size())
        return {};
    for (size_t i = 0; i < prefix.size(); ++i)
        if (std::tolower(static_cast<unsigned char>(header[i])) != prefix[i])
            return {};
    std::string token = header.substr(prefix.size());
    token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](unsigned char c) { return !std::isspace(c); }));
    token.erase(std::find_if(token.rbegin(), token.rend(), [](unsigned char c) { return !std::isspace(c); }).base(),
                token.end());
    return token;
}
}  // namespace utils
