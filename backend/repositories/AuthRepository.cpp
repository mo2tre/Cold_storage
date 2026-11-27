#include "repositories/AuthRepository.h"

#include "repositories/Db.h"

namespace repositories
{
namespace
{
using drogon::orm::DrogonDbException;
using drogon::orm::Result;

std::string toIsoUtc(std::string s)
{
    if (s.size() > 10 && s[10] == ' ')
        s[10] = 'T';
    return s + "Z";
}

RepoError toRepoError(const DrogonDbException& e)
{
    RepoError err;
    err.message = e.base().what();
    err.duplicate = err.message.find("Duplicate entry") != std::string::npos;
    return err;
}
}  // namespace

void AuthRepository::findUserByUsername(const std::string& username,
                                        std::function<void(std::optional<UserRecord>)> onOk, ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        "SELECT u.id, u.username, u.full_name, u.password_hash, u.is_active, r.name AS role_name "
        "FROM users u JOIN roles r ON r.id = u.role_id WHERE u.username = ?",
        [=](const Result& res) {
            if (res.empty())
            {
                onOk(std::nullopt);
                return;
            }
            UserRecord u;
            u.id = res[0]["id"].as<uint64_t>();
            u.username = res[0]["username"].as<std::string>();
            u.fullName = res[0]["full_name"].as<std::string>();
            u.passwordHash = res[0]["password_hash"].as<std::string>();
            u.roleName = res[0]["role_name"].as<std::string>();
            u.isActive = res[0]["is_active"].as<int>() != 0;
            onOk(u);
        },
        [=](const DrogonDbException& e) { onErr(toRepoError(e)); }, username);
}

void AuthRepository::permissionsForUser(uint64_t userId, std::function<void(std::vector<std::string>)> onOk,
                                        ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        "SELECT p.code FROM permissions p "
        "JOIN role_permissions rp ON rp.permission_id = p.id "
        "JOIN users u ON u.role_id = rp.role_id WHERE u.id = ? ORDER BY p.code",
        [=](const Result& res) {
            std::vector<std::string> codes;
            for (const auto& row : res)
                codes.push_back(row["code"].as<std::string>());
            onOk(codes);
        },
        [=](const DrogonDbException& e) { onErr(toRepoError(e)); }, userId);
}

void AuthRepository::createSession(uint64_t userId, const std::string& tokenHash, int validHours,
                                   std::function<void(std::string)> onOk, ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        "INSERT INTO sessions (user_id, token_hash, expires_at) "
        "VALUES (?, ?, DATE_ADD(UTC_TIMESTAMP(3), INTERVAL ? HOUR))",
        [=](const Result& ins) {
            const uint64_t sessionId = ins.insertId();
            db->execSqlAsync(
                "SELECT expires_at FROM sessions WHERE id = ?",
                [=](const Result& res) {
                    const std::string expires = res.empty() ? std::string() : res[0]["expires_at"].as<std::string>();
                    // Best effort: a failure here must not fail the login.
                    db->execSqlAsync(
                        "UPDATE users SET last_login_at = UTC_TIMESTAMP(3) WHERE id = ?", [](const Result&) {},
                        [](const DrogonDbException&) {}, userId);
                    onOk(toIsoUtc(expires));
                },
                [=](const DrogonDbException& e) { onErr(toRepoError(e)); }, sessionId);
        },
        [=](const DrogonDbException& e) { onErr(toRepoError(e)); }, userId, tokenHash, validHours);
}

void AuthRepository::findSession(const std::string& tokenHash, std::function<void(std::optional<SessionUser>)> onOk,
                                 ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        "SELECT u.id, u.username, u.full_name, r.name AS role_name, s.expires_at "
        "FROM sessions s JOIN users u ON u.id = s.user_id JOIN roles r ON r.id = u.role_id "
        "WHERE s.token_hash = ? AND s.revoked_at IS NULL AND s.expires_at > UTC_TIMESTAMP(3) AND u.is_active = 1",
        [=](const Result& res) {
            if (res.empty())
            {
                onOk(std::nullopt);
                return;
            }
            SessionUser su;
            su.id = res[0]["id"].as<uint64_t>();
            su.username = res[0]["username"].as<std::string>();
            su.fullName = res[0]["full_name"].as<std::string>();
            su.roleName = res[0]["role_name"].as<std::string>();
            su.expiresAt = toIsoUtc(res[0]["expires_at"].as<std::string>());
            onOk(su);
        },
        [=](const DrogonDbException& e) { onErr(toRepoError(e)); }, tokenHash);
}

void AuthRepository::revokeSession(const std::string& tokenHash, std::function<void()> onOk, ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        "UPDATE sessions SET revoked_at = UTC_TIMESTAMP(3) WHERE token_hash = ? AND revoked_at IS NULL",
        [=](const Result&) { onOk(); }, [=](const DrogonDbException& e) { onErr(toRepoError(e)); }, tokenHash);
}
}  // namespace repositories
