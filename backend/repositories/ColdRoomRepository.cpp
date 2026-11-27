#include "repositories/ColdRoomRepository.h"

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
    "id, code, name, capacity_kg, min_temp_c, max_temp_c, min_humidity_pct, max_humidity_pct, is_active, "
    "created_at, updated_at";

const char* kWhere = " WHERE (? = '' OR name LIKE ? OR code LIKE ?) AND (? < 0 OR is_active = ?)";

Json::Value rowToJson(const Row& row)
{
    Json::Value j;
    j["id"] = static_cast<Json::UInt64>(row["id"].as<uint64_t>());
    j["code"] = row["code"].as<std::string>();
    j["name"] = row["name"].as<std::string>();
    j["capacity_kg"] = decimalOrNull(row, "capacity_kg");
    j["min_temp_c"] = decimalOrNull(row, "min_temp_c");
    j["max_temp_c"] = decimalOrNull(row, "max_temp_c");
    j["min_humidity_pct"] = decimalOrNull(row, "min_humidity_pct");
    j["max_humidity_pct"] = decimalOrNull(row, "max_humidity_pct");
    j["is_active"] = row["is_active"].as<int>() != 0;
    j["created_at"] = isoUtc(row["created_at"].as<std::string>());
    j["updated_at"] = isoUtc(row["updated_at"].as<std::string>());
    return j;
}

std::string optDecimal(const std::optional<double>& v)
{
    return v ? validation::formatDecimal(*v, 2) : std::string();
}

// Ids are validated integers, so a comma-separated list is safe to bind and use with FIND_IN_SET.
std::string toCsv(const std::vector<uint64_t>& ids)
{
    std::string csv;
    for (size_t i = 0; i < ids.size(); ++i)
    {
        if (i)
            csv += ',';
        csv += std::to_string(ids[i]);
    }
    return csv;
}
}  // namespace

void ColdRoomRepository::list(uint64_t page, uint64_t pageSize, const std::string& search, int activeFilter,
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
        std::string("SELECT COUNT(*) AS c FROM cold_rooms") + kWhere,
        [=](const Result& countRes) {
            const uint64_t total = countRes.empty() ? 0 : countRes[0]["c"].as<uint64_t>();
            db->execSqlAsync(
                std::string("SELECT ") + kColumns + " FROM cold_rooms" + kWhere +
                    " ORDER BY code ASC, id ASC LIMIT ? OFFSET ?",
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

void ColdRoomRepository::findById(uint64_t id, std::function<void(std::optional<Json::Value>)> onOk, ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        std::string("SELECT ") + kColumns + " FROM cold_rooms WHERE id = ?",
        [=](const Result& res) {
            if (res.empty())
                onOk(std::nullopt);
            else
                onOk(rowToJson(res[0]));
        },
        [=](const DrogonDbException& e) { onErr(makeRepoError(e)); }, id);
}

void ColdRoomRepository::create(const dto::ColdRoomInput& in, std::function<void(Json::Value)> onOk, ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        "INSERT INTO cold_rooms (code, name, capacity_kg, min_temp_c, max_temp_c, min_humidity_pct, "
        "max_humidity_pct, is_active) VALUES (?, ?, ?, ?, ?, NULLIF(?, ''), NULLIF(?, ''), ?)",
        [=](const Result& res) {
            findById(
                res.insertId(),
                [=](std::optional<Json::Value> room) {
                    if (room)
                        onOk(*room);
                    else
                        onErr(RepoError{false, "inserted cold room not found"});
                },
                onErr);
        },
        [=](const DrogonDbException& e) { onErr(makeRepoError(e)); },
        in.code, in.name, validation::formatDecimal(in.capacityKg, 3), validation::formatDecimal(in.minTempC, 2),
        validation::formatDecimal(in.maxTempC, 2), optDecimal(in.minHumidityPct), optDecimal(in.maxHumidityPct),
        static_cast<int>(in.isActive ? 1 : 0));
}

void ColdRoomRepository::update(uint64_t id, const dto::ColdRoomInput& in,
                                std::function<void(std::optional<Json::Value>)> onOk, ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        "UPDATE cold_rooms SET code = ?, name = ?, capacity_kg = ?, min_temp_c = ?, max_temp_c = ?, "
        "min_humidity_pct = NULLIF(?, ''), max_humidity_pct = NULLIF(?, ''), is_active = ? WHERE id = ?",
        [=](const Result&) { findById(id, onOk, onErr); },
        [=](const DrogonDbException& e) { onErr(makeRepoError(e)); },
        in.code, in.name, validation::formatDecimal(in.capacityKg, 3), validation::formatDecimal(in.minTempC, 2),
        validation::formatDecimal(in.maxTempC, 2), optDecimal(in.minHumidityPct), optDecimal(in.maxHumidityPct),
        static_cast<int>(in.isActive ? 1 : 0), id);
}

void ColdRoomRepository::listProductIds(uint64_t roomId, std::function<void(std::vector<uint64_t>)> onOk,
                                        ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    db->execSqlAsync(
        "SELECT product_id FROM room_products WHERE room_id = ? ORDER BY product_id",
        [=](const Result& res) {
            std::vector<uint64_t> ids;
            for (const auto& row : res)
                ids.push_back(row["product_id"].as<uint64_t>());
            onOk(ids);
        },
        [=](const DrogonDbException& e) { onErr(makeRepoError(e)); }, roomId);
}

void ColdRoomRepository::setProductIds(uint64_t roomId, const std::vector<uint64_t>& productIds,
                                       std::function<void(std::vector<uint64_t>)> onOk,
                                       std::function<void()> onUnknown, ErrorCb onErr)
{
    auto db = dbClient();
    if (!db)
    {
        onErr(noDbError());
        return;
    }
    const std::string csv = toCsv(productIds);
    const uint64_t expected = productIds.size();

    // 1) every id must be an existing product
    db->execSqlAsync(
        "SELECT COUNT(*) AS c FROM products WHERE FIND_IN_SET(id, ?)",
        [=](const Result& countRes) {
            const uint64_t found = countRes.empty() ? 0 : countRes[0]["c"].as<uint64_t>();
            if (found != expected)
            {
                onUnknown();
                return;
            }
            // 2) add the missing links, then 3) remove the links that are no longer wanted
            db->execSqlAsync(
                "INSERT IGNORE INTO room_products (room_id, product_id) "
                "SELECT ?, id FROM products WHERE FIND_IN_SET(id, ?)",
                [=](const Result&) {
                    db->execSqlAsync(
                        "DELETE FROM room_products WHERE room_id = ? AND NOT FIND_IN_SET(product_id, ?)",
                        [=](const Result&) { listProductIds(roomId, onOk, onErr); },
                        [=](const DrogonDbException& e) { onErr(makeRepoError(e)); }, roomId, csv);
                },
                [=](const DrogonDbException& e) { onErr(makeRepoError(e)); }, roomId, csv);
        },
        [=](const DrogonDbException& e) { onErr(makeRepoError(e)); }, csv);
}
}  // namespace repositories
