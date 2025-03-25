#include "injector.h"
#include <string>
#include "platform.h"
#include <iostream>
#include <fstream>
#include <cstring>

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

Injector::Injector(const std::string &applicationName, const std::string &dllPath)
{
    InjectDLL(GetProcessIDByName(applicationName.c_str()), dllPath.c_str());
}
#if os == 0
DWORD Injector::GetProcessIDByName(const char *processName)
{
    DWORD pid = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnap, &pe))
    {
        do
        {
            if (_stricmp(pe.szExeFile, processName) == 0)
            {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return pid;
}
bool Injector::InjectDLL(DWORD processID, const char *dllPath)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess)
    {
        std::cerr << "Failed to open process: " << processID << std::endl;
        return false;
    }

    LPVOID allocMem = VirtualAllocEx(hProcess, nullptr, strlen(dllPath));
    if (!allocMem)
    {
        std::cerr << "Memory allocation failed." << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, allocMem, dllPath, strlen(dllPath) + 1, nullptr))
    {
        std::cerr << "Failed to write memory" << std::endl;
        VirutalFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"), allocMem, 0, nullptr);
    if (!hThread)
    {
        std::cerr << "Failed to create remote thread" << std::endl;
        VirutalFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    closeHandle(hProcess);
    return true;
}
#elif os == 1
pid_t Injector::GetProcessIDByName(const char *processName)
{
    DIR *dir = opendir("/proc");
    if (!dir)
        return 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0]))
        {
            std::string pidPath = std::string("/proc/") + entry->d_name + "/comm";
            std::ifstream file(pidPath);
            if (file)
            {
                std::string name;
                std::getline(file, name);
                if (name == processName)
                {
                    closedir(dir);
                    return std::stoi(entry->d_name);
                }
            }
        }
    }
    closedir(dir);
    return 0;
}

bool Injector::InjectDLL(pid_t processID, const char *dllPath)
{
    if (ptrace(PTRACE_ATTACH, processID, nullptr, nullptr) == -1)
    {
        perror("ptrace attach");
        return false;
    }

    waitpid(processID, nullptr, 0);

    void *handle = dlopen(dllPath, RTLD_NOW);
    if (!handle)
    {
        std::cerr << "Failed to load shared object: " << dllPath << std::endl;
        ptrace(PTRACE_DETACH, processID, nullptr, nullptr);
        return false;
    }

    ptrace(PTRACE_DETACH, processID, nullptr, nullptr);
    return true;
}
#elif os == 2
pid_t Injector::GetProcessIDByName(const char *processName)
{
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    size_t size;
    sysctl(mib, 4, nullptr, &size, nullptr, 0);

    struct kinfo_proc *procs = (kinfo_proc *)malloc(size);
    if (!procs)
        return 0;

    sysctl(mib, 4, procs, &size, nullptr, 0);
    int procCout = size / sizeof(struct kinfo_proc);

    for (int i = 0; i < procCount; i++)
    {
        char name[PROC_PIDPATHINFO_MAXSIZE];
        proc_name(procs[i].kp_proc.p_pid, name, sizeof(name));
        if (strcmp(name, processName) == 0)
        {
            pid_t pid = procs[i].kp_proc.p_pid;
            free(procs);
            return pid;
        }
    }
    free(procs);
    return 0;
}
bool Injector::InjectDLL(pid_t processID, const char *dylibPath)
{
    task_t task;
    if (task_for_pid(mach_task_self(), pid, &task) != KERN_SUCCESS)
    {
        std::cerr << "Failed to get task for process: " << processID << std::endl;
        return false;
    }

    mach_vm_address_t remoteMemory;
    if (mach_vm_allocate(task, &remoteMemory, strlen(dylibPath) + 1, VM_FLAGS_ANYWHERE) != KERN_SUCCESS)
    {
        std::cerr << "Memory allocation failed" << std::endl;
        return false;
    }

    if (mach_vm_write(task, remoteMemory, (vm_offset_t)dylibPath, (mach_msg_type_number_t)strlen(dylibPath) + 1) != KERN_SUCCESS)
    {
        std::cerr << "Failed to write memory" << std::endl;
        return false;
    }

    std::cout << "Dylib successfully injected" << std::endl;
    return true;
}
#endif