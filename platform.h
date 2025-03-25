#ifndef PLATFORM_H
#define PLATFORM_H

#if _WIN32
#define os 0
#elif __linux__
#define os 1
#elif __APPLE__
#define os 2
#endif

#endif