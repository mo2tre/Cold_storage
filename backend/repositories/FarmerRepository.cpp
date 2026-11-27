#include "repositories/FarmerRepository.h"

#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>

#include <memory>

namespace repositories
{
namespace
{
using drogon::orm::DrogonDbException;
using drogon::orm::Result;
using drogon::orm::Row;

constexpr const char* kColumns =
    "id, code, full_name, phone, email, address, identifier, notes, is_active, created_at, updated_at";

// "2026-11-26 21:00:00.123" (stored in UTC) -> "2026-11-26T21:00:00.123Z"
std::string toIsoUtc(std::string s)
{
    if (s.size() > 10 && s[10] == ' ')
        s[10] = 'T';
    return s + "Z";
}

void setNullable(Json::Value& j, const char* key, const Row& row, const char* column)
{
    j[key] = row[column].isNull() ? Json::Value(Json::nullValue) : Json::Value(row[column].as<std::string>());
}

Json::Value rowToJson(const Row& row)
{
    Json::Value j;
    j["id"] = static_cast<Json::UInt64>(row["id"].as<uint64_t>());
    j["code"] = row["code"].as<std::string>();
    j["full_name"] = row["full_name"].as<std::string>();
    setNullable(j, "phone", row, "phone");
    setNullable(j, "email", row, "email");
    setNullable(j, "address", row, "address");
    setNullable(j, "identifier", row, "identifier");
    setNullable(j, "notes", row, "notes");
    j["is_active"] = row["is_active"].as<int>() != 0;
    j["created_at"] = toIsoUtc(row["created_at"].as<std::string>());
    j["updated_at"] = toIsoUtc(row["updated_at"].as<std::string>());
    return j;
}

// Optional fields are bound as strings; the SQL turns '' into NULL with NULLIF(?, '').
std::string orEmpty(const std::shared_ptr<std::string>& v)
{
    return v ? *v : std::string();
}

RepoError toRepoError(const DrogonDbException& e)
{
    RepoError err;
    err.message = e.base().what();
    err.duplicate = err.message.find("Duplicate entry") != std::string::npos;
    return err;
}

const char* kWhere =
    " WHERE (? = '' OR full_name LIKE ? OR code LIKE ? OR phone LIKE ?) AND (? < 0 OR is_active = ?)";
}  // namespace

void FarmerRepository::list(uint64_t page, uint64_t pageSize, const std::string& search, int activeFilter,
                            std::function<void(Json::Value, uint64_t)> onOk, ErrorCb onErr)
{
    auto db = drogon::app().getDbClient();
    const std::string pattern = "%" + search + "%";
    const uint64_t offset = (page - 1) * pageSize;

    db->execSqlAsync(
        std::string("SELECT COUNT(*) AS c FROM farmers") + kWhere,
        [=](const Result& countRes) {
            const uint64_t total = countRes.empty() ? 0 : countRes[0]["c"].as<uint64_t>();
            drogon::app().getDbClient()->execSqlAsync(
                std::string("SELECT ") + kColumns + " FROM farmers" + kWhere +
                    " ORDER BY full_name ASC, id ASC LIMIT ? OFFSET ?",
                [=](const Result& res) {
                    Json::Value items(Json::arrayValue);
                    for (const auto& row : res)
                        items.append(rowToJson(row));
                    onOk(items, total);
                },
                [=](const DrogonDbException& e) { onErr(toRepoError(e)); },
                search, pattern, pattern, pattern, activeFilter, activeFilter, pageSize, offset);
        },
        [=](const DrogonDbException& e) { onErr(toRepoError(e)); },
        search, pattern, pattern, pattern, activeFilter, activeFilter);
}

void FarmerRepository::findById(uint64_t id, std::function<void(std::optional<Json::Value>)> onOk, ErrorCb onErr)
{
    drogon::app().getDbClient()->execSqlAsync(
        std::string("SELECT ") + kColumns + " FROM farmers WHERE id = ?",
        [=](const Result& res) {
            if (res.empty())
                onOk(std::nullopt);
            else
                onOk(rowToJson(res[0]));
        },
        [=](const DrogonDbException& e) { onErr(toRepoError(e)); }, id);
}

void FarmerRepository::create(const dto::FarmerInput& in, std::function<void(Json::Value)> onOk, ErrorCb onErr)
{
    drogon::app().getDbClient()->execSqlAsync(
        "INSERT INTO farmers (code, full_name, phone, email, address, identifier, notes, is_active) "
        "VALUES (?, ?, NULLIF(?, ''), NULLIF(?, ''), NULLIF(?, ''), NULLIF(?, ''), NULLIF(?, ''), ?)",
        [=](const Result& res) {
            const uint64_t newId = res.insertId();
            findById(
                newId,
                [=](std::optional<Json::Value> farmer) {
                    if (farmer)
                        onOk(*farmer);
                    else
                        onErr(RepoError{false, "inserted farmer not found"});
                },
                onErr);
        },
        [=](const DrogonDbException& e) { onErr(toRepoError(e)); },
        in.code, in.fullName, orEmpty(in.phone), orEmpty(in.email), orEmpty(in.address),
        orEmpty(in.identifier), orEmpty(in.notes), static_cast<int>(in.isActive ? 1 : 0));
}

void FarmerRepository::update(uint64_t id, const dto::FarmerInput& in,
                              std::function<void(std::optional<Json::Value>)> onOk, ErrorCb onErr)
{
    drogon::app().getDbClient()->execSqlAsync(
        "UPDATE farmers SET code = ?, full_name = ?, phone = NULLIF(?, ''), email = NULLIF(?, ''), "
        "address = NULLIF(?, ''), identifier = NULLIF(?, ''), notes = NULLIF(?, ''), is_active = ? WHERE id = ?",
        [=](const Result&) {
            // affectedRows is 0 when nothing changed, so re-read to know whether the id exists.
            findById(id, onOk, onErr);
        },
        [=](const DrogonDbException& e) { onErr(toRepoError(e)); },
        in.code, in.fullName, orEmpty(in.phone), orEmpty(in.email), orEmpty(in.address),
        orEmpty(in.identifier), orEmpty(in.notes), static_cast<int>(in.isActive ? 1 : 0), id);
}
}  // namespace repositories