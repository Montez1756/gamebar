#ifndef PLATFORM_H
#define PLATFORM_H

#include <string>

// Define OS identification macros
#if _WIN32
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_LINUX 0
    #define PLATFORM_APPLE 0
    constexpr int os = 0;
    inline const std::string platform = "windows";
#elif __linux__
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX 1
    #define PLATFORM_APPLE 0
    constexpr int os = 1;
    inline const std::string platform = "linux";
#elif __APPLE__
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX 0
    #define PLATFORM_APPLE 1
    constexpr int os = 2;
    inline const std::string platform = "apple";
#else
    #error "Unknown platform"
#endif

#endif // PLATFORM_H