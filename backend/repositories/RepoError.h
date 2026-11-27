#pragma once

#include <functional>
#include <string>

namespace repositories
{
struct RepoError
{
    bool duplicate = false;  // unique constraint violated
    std::string message;     // for server logs only, never sent to clients
};

using ErrorCb = std::function<void(const RepoError&)>;
}  // namespace repositories
