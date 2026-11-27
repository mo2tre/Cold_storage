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

inline RepoError makeRepoError(const drogon::orm::DrogonDbException& e)
{
    RepoError err;
    err.message = e.base().what();
    err.duplicate = err.message.find("Duplicate entry") != std::string::npos;
    return err;
}

inline RepoError noDbError()
{
    return RepoError{false, "database client not available"};
}
}  // namespace repositories
