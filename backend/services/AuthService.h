#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace services
{
struct AuthUser
{
    uint64_t id = 0;
    std::string username;
    std::string fullName;
    std::string role;
    std::vector<std::string> permissions;
    std::string expiresAt;  // ISO 8601 UTC
};

class AuthService
{
  public:
    using ResponseCb = std::function<void(const drogon::HttpResponsePtr&)>;

    static void login(const drogon::HttpRequestPtr& req, ResponseCb cb);
    static void logout(const drogon::HttpRequestPtr& req, ResponseCb cb);
    static void me(const drogon::HttpRequestPtr& req, ResponseCb cb);

    // Checks "Authorization: Bearer <token>" and the permission (empty = any logged-in user).
    // Calls onOk with the user when allowed; otherwise answers 401/403/500 through cb.
    static void authorize(const drogon::HttpRequestPtr& req, const std::string& permission,
                          std::function<void(const AuthUser&)> onOk, ResponseCb cb);
};
}  // namespace services
