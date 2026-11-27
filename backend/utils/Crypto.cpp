#include "utils/Crypto.h"

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <cstdlib>
#include <stdexcept>
#include <vector>

namespace utils
{
namespace
{
constexpr int kIterations = 600000;  // OWASP guidance for PBKDF2-HMAC-SHA256
constexpr size_t kSaltBytes = 16;
constexpr size_t kHashBytes = 32;

std::string toHex(const unsigned char* data, size_t len)
{
    static const char* digits = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i)
    {
        out.push_back(digits[data[i] >> 4]);
        out.push_back(digits[data[i] & 0x0F]);
    }
    return out;
}

int hexValue(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

bool fromHex(const std::string& s, std::vector<unsigned char>& out)
{
    if (s.empty() || s.size() % 2 != 0)
        return false;
    out.clear();
    out.reserve(s.size() / 2);
    for (size_t i = 0; i < s.size(); i += 2)
    {
        int hi = hexValue(s[i]);
        int lo = hexValue(s[i + 1]);
        if (hi < 0 || lo < 0)
            return false;
        out.push_back(static_cast<unsigned char>(hi * 16 + lo));
    }
    return true;
}

std::vector<unsigned char> randomBytes(size_t n)
{
    std::vector<unsigned char> bytes(n);
    if (RAND_bytes(bytes.data(), static_cast<int>(n)) != 1)
        throw std::runtime_error("RAND_bytes failed");
    return bytes;
}

std::vector<unsigned char> pbkdf2(const std::string& password, const unsigned char* salt, size_t saltLen,
                                  int iterations, size_t outLen)
{
    std::vector<unsigned char> out(outLen);
    if (PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()), salt, static_cast<int>(saltLen),
                          iterations, EVP_sha256(), static_cast<int>(outLen), out.data()) != 1)
        throw std::runtime_error("PBKDF2 failed");
    return out;
}
}  // namespace

std::string hashPassword(const std::string& password)
{
    auto salt = randomBytes(kSaltBytes);
    auto hash = pbkdf2(password, salt.data(), salt.size(), kIterations, kHashBytes);
    return "pbkdf2_sha256$" + std::to_string(kIterations) + "$" + toHex(salt.data(), salt.size()) + "$" +
           toHex(hash.data(), hash.size());
}

bool verifyPassword(const std::string& password, const std::string& stored)
{
    // scheme$iterations$salt$hash
    std::vector<std::string> parts;
    size_t start = 0;
    while (true)
    {
        size_t pos = stored.find('$', start);
        if (pos == std::string::npos)
        {
            parts.push_back(stored.substr(start));
            break;
        }
        parts.push_back(stored.substr(start, pos - start));
        start = pos + 1;
    }
    if (parts.size() != 4 || parts[0] != "pbkdf2_sha256")
        return false;

    char* end = nullptr;
    long iterations = std::strtol(parts[1].c_str(), &end, 10);
    if (end == parts[1].c_str() || *end != '\0' || iterations < 1000 || iterations > 10000000)
        return false;

    std::vector<unsigned char> salt, expected;
    if (!fromHex(parts[2], salt) || !fromHex(parts[3], expected))
        return false;

    try
    {
        auto actual = pbkdf2(password, salt.data(), salt.size(), static_cast<int>(iterations), expected.size());
        return actual.size() == expected.size() && CRYPTO_memcmp(actual.data(), expected.data(), actual.size()) == 0;
    }
    catch (...)
    {
        return false;
    }
}

std::string randomToken()
{
    auto bytes = randomBytes(32);
    return toHex(bytes.data(), bytes.size());
}

std::string sha256Hex(const std::string& data)
{
    unsigned char md[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.data()), data.size(), md);
    return toHex(md, sizeof(md));
}
}  // namespace utils
