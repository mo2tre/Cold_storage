#pragma once

#include <string>

namespace utils
{
// PBKDF2-HMAC-SHA256 with a random 16-byte salt. Stored format:
//   pbkdf2_sha256$<iterations>$<salt_hex>$<hash_hex>
std::string hashPassword(const std::string& password);
bool verifyPassword(const std::string& password, const std::string& stored);

// 256-bit random token as 64 hex characters.
std::string randomToken();

// Lowercase hex SHA-256 (64 chars). Used to store session tokens.
std::string sha256Hex(const std::string& data);
}  // namespace utils
