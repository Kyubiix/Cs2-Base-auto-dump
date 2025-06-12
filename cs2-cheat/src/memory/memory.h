#pragma once
#include <cstdint>
#include <Windows.h>
#include <TlHelp32.h>
#include <string_view>
#include <iostream>

#define WIN32_LEAN_AND_MEAN

class Memory {
private:
    std::uintptr_t processId = 0;
    HANDLE processHandle = nullptr;

public:
    // Constructor: Finds and opens the target process
    Memory(const std::string_view processName) noexcept {
        PROCESSENTRY32 entry = {};
        entry.dwSize = sizeof(PROCESSENTRY32);

        const auto snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapShot == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Failed to create process snapshot." << std::endl;
            return;
        }

        while (::Process32Next(snapShot, &entry)) {
            if (!processName.compare(entry.szExeFile)) {
                processId = entry.th32ProcessID;
                processHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
                if (!processHandle || processHandle == INVALID_HANDLE_VALUE) {
                    std::cerr << "Error: Failed to open process: " << processName << std::endl;
                }
                break;
            }
        }

        if (snapShot) {
            ::CloseHandle(snapShot);
        }

        if (!processId) {
            std::cerr << "Error: Process not found: " << processName << std::endl;
        }
    }

    // Destructor: Closes the process handle
    ~Memory() {
        if (processHandle) {
            ::CloseHandle(processHandle);
        }
    }

    // Get the base address of a module
    const std::uintptr_t GetModuleAddress(const std::string_view moduleName) const noexcept {
        MODULEENTRY32 entry = {};
        entry.dwSize = sizeof(MODULEENTRY32);

        const auto snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
        if (snapShot == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Failed to create module snapshot." << std::endl;
            return 0;
        }

        std::uintptr_t result = 0;

        while (::Module32Next(snapShot, &entry)) {
            if (!moduleName.compare(entry.szModule)) {
                result = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
                break;
            }
        }

        if (snapShot) {
            ::CloseHandle(snapShot);
        }

        if (!result) {
            std::cerr << "Error: Module not found: " << moduleName << std::endl;
        }

        return result;
    }

    bool IsValid() const noexcept {
        return processHandle != nullptr && processHandle != INVALID_HANDLE_VALUE;
    }

    // Read memory
    template <typename T>
    constexpr T Read(const std::uintptr_t& address) const noexcept {
        T value = {};
        if (!::ReadProcessMemory(processHandle, reinterpret_cast<const void*>(address), &value, sizeof(T), NULL)) {
            std::cerr << "Error: Failed to read memory at address: 0x" << std::hex << address << std::endl;
        }
        return value;
    }

    // Write memory
    template <typename T>
    constexpr void Write(const std::uintptr_t& address, const T& value) const noexcept {
        if (!::WriteProcessMemory(processHandle, reinterpret_cast<void*>(address), &value, sizeof(T), NULL)) {
            std::cerr << "Error: Failed to write memory at address: 0x" << std::hex << address << std::endl;
        }
    }
};
