#ifndef INJECTOR_H
#define INJECTOR_H

#include <string>
#include "platform.h"
#include <iostream>
#include <fstream>

#if os == 0
#include <windows.h>
#include <tlhelp32.h>
#elif os == 1
#include <dirent.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#elif os == 2
#include <sys/sysctl.h>
#include <libproc.h>
#include <mach/mach.h>
#endif

// #ifndef DWORD
// typedef unsigned long DWORD;
// #endif
class Injector
{
private:
#if os == 0
    DWORD GetProcessIDByName(const char *processName);
    bool InjectDLL(DWORD processID, const char *dllPath);
#else
    pid_t GetProcessIDByName(const char *processName);
    bool InjectDLL(pid_t processID, const char *dllPath);
#endif


public:
    Injector(const std::string &applicationName, const std::string &dllPath);
};
#endif