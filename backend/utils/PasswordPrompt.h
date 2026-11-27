#pragma once

#include <string>

namespace utils
{
// Prints the prompt and reads a line from the console without echoing it (Windows).
std::string readPasswordHidden(const std::string& prompt);
}  // namespace utils
