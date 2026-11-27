#pragma once

#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>

#include "repositories/RepoError.h"

namespace repositories
{
// Returns nullptr (instead of crashing) when the database client is not available.
inline drogon::orm::DbClientPtr dbClient()
{
    try
    {
        return drogon::app().getDbClient();
    }
    catch (...)
    {
        return nullptr;
    }
}

inline RepoError noDbError()
{
    return RepoError{false, "database client not available"};
}
}  // namespace repositories
