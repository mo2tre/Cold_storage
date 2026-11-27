#include "utils/PasswordPrompt.h"

#include <iostream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace utils
{
std::string readPasswordHidden(const std::string& prompt)
{
    std::cout << prompt << std::flush;
    std::string line;
#ifdef _WIN32
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    const bool isConsole = GetConsoleMode(in, &mode) != 0;
    if (isConsole)
        SetConsoleMode(in, mode & ~ENABLE_ECHO_INPUT);
    std::getline(std::cin, line);
    if (isConsole)
        SetConsoleMode(in, mode);
#else
    std::getline(std::cin, line);
#endif
    std::cout << "\n";
    return line;
}
}  // namespace utils
