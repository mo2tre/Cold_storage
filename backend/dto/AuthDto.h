#pragma once

#include <json/json.h>

#include <string>

namespace dto
{
struct LoginInput
{
    std::string username;
    std::string password;
};

// Returns an array of {field, message}; empty means valid.
Json::Value parseLoginInput(const Json::Value& body, LoginInput& out);
}  // namespace dto
