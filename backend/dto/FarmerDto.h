#pragma once

#include <json/json.h>

#include <memory>
#include <string>

namespace dto
{
// Validated farmer fields. A null shared_ptr means SQL NULL.
struct FarmerInput
{
    std::string code;
    std::string fullName;
    std::shared_ptr<std::string> phone;
    std::shared_ptr<std::string> email;
    std::shared_ptr<std::string> address;
    std::shared_ptr<std::string> identifier;
    std::shared_ptr<std::string> notes;
    bool isActive = true;
};

// Parses and validates a JSON body (POST /farmers, PUT /farmers/{id}).
// Returns an array of {field, message}; empty means the input is valid.
Json::Value parseFarmerInput(const Json::Value& body, FarmerInput& out);
}  // namespace dto
