#include "repositories/ProductRepository.h"

#include "repositories/Db.h"
#include "repositories/RowUtil.h"
#include "utils/Validation.h"

namespace repositories
{
namespace
{
using drogon::orm::DrogonDbException;
using drogon::orm::Result;
using drogon::orm::Row;

constexpr const char* kColumns =
    "id, code, name, min_temp_c, max_temp_c, min_humidity_pct, max_humidity_pct, max_storage_days, is_active, "
    "created_at, updated_at";

const char* kWhere = " WHERE (? = '' OR name LIKE ? OR code LIKE ?) AND (? < 0 OR is_active = ?)";

Json::Value rowToJson(const Row& row)
{
    Json::Value j;
    j["id"] = static_cast<Json::UInt64>(row["id"].as<uint64_t>());
    j["code"] = row["code"].as<std::string>();
    j["name"] = row["name"].as<std::string>();
    j["min_temp_c"] = decimalOrNull(row, "min_temp_c");
    j["max_temp_c"] = decimalOrNull(row, "max_temp_c");
    j["min_humidity_pct"] = decimalOrNull(row, "min_humidity_pct");
    j["max_humidity_pct"] = decimalOrNull(row, "max_humidity_pct");
    j["max_storage_days"] = uintOrNull(row, "max_storage_days");
    j["is_active"] = row["is_active"].as<int>() != 0;
    j["created_at"] = isoUtc(row["created_at"].as<std::string>());
    j["updated_at"] = isoUtc(row["updated_at"].as<std::string>());
    return j;
}

// Optional values are bound as strings; the SQL turns '' into NULL with NULLIF(?, '').
std::string optDecimal(const std::optional<double>& v)
{
    return v ? validation::formatDecimal(*v, 2) : std::string();
}
std::string optInt(const std::optional<uint32_t>& v)
{
    return v ? std::to_string(*v) : std::string();
}
}  // namespace

void ProductRepository::list(uint64_t page, uint64_t pageSize, const std::string& search, int activeFilter,
                             std::function<void(Json::Value, uint64_t)> onOk, ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    const std::string pattern = "%" + search + "%";
    const uint64_t offset = (page - 1) * pageSize;

    db->execSqlAsync(
        std::string("SELECT COUNT(*) AS c FROM products") + kWhere,
        [=](const Result& countRes) {
            const uint64_t total = countRes.empty() ? 0 : countRes[0]["c"].as<uint64_t>();
            db->execSqlAsync(
                std::string("SELECT ") + kColumns + " FROM products" + kWhere +
                    " ORDER BY name ASC, id ASC LIMIT ? OFFSET ?",
                [=](const Result& res) {
                    Json::Value items(Json::arrayValue);
                    for (const auto& row : res)
                        items.append(rowToJson(row));
                    onOk(items, total);
                },
                [=](const DrogonDbException& e) { onErr(makeRepoError(e)); },
                search, pattern, pattern, activeFilter, activeFilter, pageSize, offset);
        },
        [=](const DrogonDbException& e) { onErr(makeRepoError(e)); },
        search, pattern, pattern, activeFilter, activeFilter);
}

void ProductRepository::findById(uint64_t id, std::function<void(std::optional<Json::Value>)> onOk, ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        std::string("SELECT ") + kColumns + " FROM products WHERE id = ?",
        [=](const Result& res) {
            if (res.empty())
                onOk(std::nullopt);
            else
                onOk(rowToJson(res[0]));
        },
        [=](const DrogonDbException& e) { onErr(makeRepoError(e)); }, id);
}

void ProductRepository::create(const dto::ProductInput& in, std::function<void(Json::Value)> onOk, ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        "INSERT INTO products (code, name, min_temp_c, max_temp_c, min_humidity_pct, max_humidity_pct, "
        "max_storage_days, is_active) VALUES (?, ?, ?, ?, NULLIF(?, ''), NULLIF(?, ''), NULLIF(?, ''), ?)",
        [=](const Result& res) {
            findById(
                res.insertId(),
                [=](std::optional<Json::Value> product) {
                    if (product)
                        onOk(*product);
                    else
                        onErr(RepoError{false, "inserted product not found"});
                },
                onErr);
        },
        [=](const DrogonDbException& e) { onErr(makeRepoError(e)); },
        in.code, in.name, validation::formatDecimal(in.minTempC, 2), validation::formatDecimal(in.maxTempC, 2),
        optDecimal(in.minHumidityPct), optDecimal(in.maxHumidityPct), optInt(in.maxStorageDays),
        static_cast<int>(in.isActive ? 1 : 0));
}

void ProductRepository::update(uint64_t id, const dto::ProductInput& in,
                               std::function<void(std::optional<Json::Value>)> onOk, ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        "UPDATE products SET code = ?, name = ?, min_temp_c = ?, max_temp_c = ?, "
        "min_humidity_pct = NULLIF(?, ''), max_humidity_pct = NULLIF(?, ''), max_storage_days = NULLIF(?, ''), "
        "is_active = ? WHERE id = ?",
        [=](const Result&) { findById(id, onOk, onErr); },
        [=](const DrogonDbException& e) { onErr(makeRepoError(e)); },
        in.code, in.name, validation::formatDecimal(in.minTempC, 2), validation::formatDecimal(in.maxTempC, 2),
        optDecimal(in.minHumidityPct), optDecimal(in.maxHumidityPct), optInt(in.maxStorageDays),
        static_cast<int>(in.isActive ? 1 : 0), id);
}
}  // namespace repositories
