#include "services/AuthService.h"

#include <drogon/drogon.h>

#include <algorithm>

#include "dto/AuthDto.h"
#include "repositories/AuthRepository.h"
#include "utils/ApiResponse.h"
#include "utils/Crypto.h"
#include "utils/HttpUtils.h"

namespace services
{
namespace
{
using repositories::AuthRepository;
using repositories::RepoError;
using repositories::SessionUser;
using repositories::UserRecord;

constexpr int kSessionHours = 8;

repositories::ErrorCb dbErrorHandler(AuthService::ResponseCb cb)
{
    return [cb](const RepoError& err) {
        LOG_ERROR << "Database error: " << err.message;
        cb(api::internalError());
    };
}

Json::Value userToJson(const AuthUser& u)
{
    Json::Value j;
    j["id"] = static_cast<Json::UInt64>(u.id);
    j["username"] = u.username;
    j["full_name"] = u.fullName;
    j["role"] = u.role;
    Json::Value perms(Json::arrayValue);
    for (const auto& p : u.permissions)
        perms.append(p);
    j["permissions"] = perms;
    return j;
}

AuthUser makeAuthUser(const SessionUser& su, std::vector<std::string> perms)
{
    AuthUser u;
    u.id = su.id;
    u.username = su.username;
    u.fullName = su.fullName;
    u.role = su.roleName;
    u.permissions = std::move(perms);
    u.expiresAt = su.expiresAt;
    return u;
}
}  // namespace

void AuthService::login(const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    auto body = req->getJsonObject();
    if (!body)
    {
        cb(api::badRequest("Request body must be valid JSON"));
        return;
    }
    dto::LoginInput input;
    Json::Value details = dto::parseLoginInput(*body, input);
    if (!details.empty())
    {
        cb(api::validationError("Invalid login data", details));
        return;
    }

    const auto onDbError = dbErrorHandler(cb);
    AuthRepository::findUserByUsername(
        input.username,
        [cb, input, onDbError](std::optional<UserRecord> user) {
            // Same answer and similar timing whether the user exists, is inactive, or the password is wrong.
            static const std::string dummyHash = utils::hashPassword("dummy-password-for-timing");
            const bool passwordOk = utils::verifyPassword(input.password, user ? user->passwordHash : dummyHash);
            if (!user || !user->isActive || !passwordOk)
            {
                cb(api::unauthorized("Invalid username or password"));
                return;
            }

            const std::string token = utils::randomToken();
            const std::string tokenHash = utils::sha256Hex(token);
            const UserRecord u = *user;
            AuthRepository::permissionsForUser(
                u.id,
                [cb, u, token, tokenHash, onDbError](std::vector<std::string> perms) {
                    AuthRepository::createSession(
                        u.id, tokenHash, kSessionHours,
                        [cb, u, token, perms](std::string expiresAt) {
                            AuthUser au;
                            au.id = u.id;
                            au.username = u.username;
                            au.fullName = u.fullName;
                            au.role = u.roleName;
                            au.permissions = perms;

                            Json::Value data;
                            data["token"] = token;
                            data["token_type"] = "Bearer";
                            data["expires_at"] = expiresAt;
                            data["user"] = userToJson(au);
                            cb(api::ok(data, "Login successful"));
                        },
                        onDbError);
                },
                onDbError);
        },
        onDbError);
}

void AuthService::logout(const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    const std::string token = utils::bearerToken(req);
    if (token.empty())
    {
        cb(api::unauthorized());
        return;
    }
    AuthRepository::revokeSession(
        utils::sha256Hex(token), [cb]() { cb(api::ok(Json::Value(Json::objectValue), "Logged out")); },
        dbErrorHandler(cb));
}

void AuthService::me(const drogon::HttpRequestPtr& req, ResponseCb cb)
{
    authorize(
        req, "",
        [cb](const AuthUser& u) {
            Json::Value data = userToJson(u);
            data["expires_at"] = u.expiresAt;
            cb(api::ok(data));
        },
        cb);
}

void AuthService::authorize(const drogon::HttpRequestPtr& req, const std::string& permission,
                            std::function<void(const AuthUser&)> onOk, ResponseCb cb)
{
    const std::string token = utils::bearerToken(req);
    if (token.empty())
    {
        cb(api::unauthorized());
        return;
    }

    const auto onDbError = dbErrorHandler(cb);
    AuthRepository::findSession(
        utils::sha256Hex(token),
        [permission, onOk, cb, onDbError](std::optional<SessionUser> session) {
            if (!session)
            {
                cb(api::unauthorized("Invalid or expired token"));
                return;
            }
            const SessionUser su = *session;
            AuthRepository::permissionsForUser(
                su.id,
                [permission, onOk, cb, su](std::vector<std::string> perms) {
                    if (!permission.empty() && std::find(perms.begin(), perms.end(), permission) == perms.end())
                    {
                        cb(api::forbidden("You do not have permission to do this"));
                        return;
                    }
                    onOk(makeAuthUser(su, std::move(perms)));
                },
                onDbError);
        },
        onDbError);
}
}  // namespace services
