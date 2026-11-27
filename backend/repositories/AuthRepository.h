#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "repositories/RepoError.h"

namespace repositories
{
struct UserRecord
{
    uint64_t id = 0;
    std::string username;
    std::string fullName;
    std::string passwordHash;
    std::string roleName;
    bool isActive = false;
};

struct SessionUser
{
    uint64_t id = 0;
    std::string username;
    std::string fullName;
    std::string roleName;
    std::string expiresAt;  // ISO 8601 UTC
};

class AuthRepository
{
  public:
    static void findUserByUsername(const std::string& username, std::function<void(std::optional<UserRecord>)> onOk,
                                   ErrorCb onErr);

    static void permissionsForUser(uint64_t userId, std::function<void(std::vector<std::string>)> onOk, ErrorCb onErr);

    // Stores the token hash, updates last_login_at, and returns the session expiry (ISO 8601 UTC).
    static void createSession(uint64_t userId, const std::string& tokenHash, int validHours,
                              std::function<void(std::string expiresAt)> onOk, ErrorCb onErr);

    // Only returns sessions that are not revoked, not expired, and belong to an active user.
    static void findSession(const std::string& tokenHash, std::function<void(std::optional<SessionUser>)> onOk,
                            ErrorCb onErr);

    static void revokeSession(const std::string& tokenHash, std::function<void()> onOk, ErrorCb onErr);
};
}  // namespace repositories
