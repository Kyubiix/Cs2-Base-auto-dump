#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <cstddef>
#include <iostream>
#include <map>
#include <string>

#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_internal.h"

// Offsets Namespace
namespace offsets {
    inline std::ptrdiff_t dwEntityList = 0x167BCC8;
    inline std::ptrdiff_t dwViewMatrix = 0x1715D90;
    inline std::ptrdiff_t dwLocalPlayer = 0x17153E8;

    inline std::ptrdiff_t dwPawnHealth = 0x808;
    inline std::ptrdiff_t dwPlayerPawn = 0x5dc;
    inline std::ptrdiff_t dwSanitizedName = 0x720;
    inline std::ptrdiff_t m_bDormant = 0xE7;
    inline std::ptrdiff_t m_iTeamNum = 0x959;
    inline std::ptrdiff_t m_vecOrigin = 0x1214;
    inline std::ptrdiff_t m_iHealth = 0x32C;

}

// Globals Namespace
namespace globals {
    HWND hwnd = nullptr;

    void UpdateOffsets() {
        struct Signature {
            const char* pattern;
            const char* mask;
            uintptr_t offset1;
            uintptr_t offset2;
        };

        // Signature to Offset Map
        std::map<std::string, Signature> signatures = {
            {"dwEntityList", {"48 8B 0D ? ? ? ? 48 89 5C 24 ? 8B", "xxx????xxxx", 0x03, 0x07}},
            {"dwViewMatrix", {"48 8D 0D ? ? ? ? 48 C1 E0 06", "xxx????xxxx", 0x03, 0x07}},
            {"dwLocalPlayer", {"48 83 3D ? ? ? ? ? 0F 95", "xxx?????xx", 0x03, 0x08}},

            // Additional Signatures
            {"UTIL_ClientPrintAll", {"48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 81 EC 70 01 ? ?", "xxxxxxxxxxxxxxxxxxx", 0x03, 0x07}},
            {"ClientPrint", {"48 85 C9 0F 84 ? ? ? ? 48 89 5C 24 ? 55", "xxxxxxxxxxxxx", 0x03, 0x07}},
            {"CCSPlayerController_SwitchTeam", {"40 56 57 48 81 EC ? ? ? ? 48 8B F9 8B F2 8B CA", "xxxxxxxxxxxxxxx", 0x03, 0x07}},
            {"CBasePlayerController_SetPawn", {"48 89 74 24 ? 55 41 54 41 55 41 56 41 57 48 8D 6C 24 ?", "xxxxxxxxxxxxxxx", 0x03, 0x07}},
            {"GiveNamedItem", {"48 89 5C 24 ? 48 89 74 24 ? 55 57 41 ? 41 ? 41 ? 48 ? ? ? ?", "xxxxxxxxxxxxx", 0x03, 0x07}},
            {"UTIL_Remove", {"48 85 C9 74 ? 48 8B D1 48 8B 0D ? ? ? ?", "xxxxxxxxxxxx", 0x03, 0x07}},
            {"Host_Say", {"44 89 4C 24 20 44 88 44 24 18", "xxxxxxxxxx", 0x03, 0x07}},
            {"CCSPlayer_WeaponServices_CanUse", {"48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ?", "xxxxxxxxxxxxx", 0x03, 0x07}},
            {"CCSPlayer_ItemServices_CanAcquire", {"48 8B C4 44 89 40 ? 48 89 50 ? 48 89 48", "xxxxxxxxxx", 0x03, 0x07}},
            {"GetCSWeaponDataFromKey", {"48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B FA 8B F1", "xxxxxxxxxxxxx", 0x03, 0x07}},
            {"CCSGameRules_TerminateRound", {"48 8B C4 4C 89 48 ? 48 89 48 ? 55 41 55", "xxxxxxxxxxxxx", 0x03, 0x07}},
        };

        // Function to Find Signature in Memory
        auto FindPattern = [](const char* moduleName, const char* pattern, const char* mask) -> uintptr_t {
            MODULEINFO moduleInfo;
            HMODULE hModule = GetModuleHandleA(moduleName);
            if (!hModule) return 0;

            GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(MODULEINFO));
            uintptr_t startAddress = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
            uintptr_t endAddress = startAddress + moduleInfo.SizeOfImage;

            size_t patternLength = strlen(mask);

            for (uintptr_t address = startAddress; address < endAddress - patternLength; ++address) {
                bool found = true;
                for (size_t i = 0; i < patternLength; ++i) {
                    if (mask[i] != '?' && pattern[i] != *reinterpret_cast<char*>(address + i)) {
                        found = false;
                        break;
                    }
                }
                if (found) return address;
            }
            return 0;
        };

        // Resolve Signatures and Update Offsets
        for (const auto& [name, signature] : signatures) {
            uintptr_t baseAddress = FindPattern("server.dll", signature.pattern, signature.mask);
            if (baseAddress) {
                uintptr_t resolvedAddress = baseAddress + signature.offset1 + signature.offset2;
                std::cout << name << " resolved at: 0x" << std::hex << resolvedAddress << std::endl;
            }
            else {
                std::cerr << name << " not found!" << std::endl;
            }
        }
    }
}

namespace toggles {
    inline bool menu = false;

    namespace hacks {
        inline bool bESP = false;
        inline bool bSnaplines = false;
        inline bool bHealth = false;
        inline bool bNames = false;
        inline bool bDistance = false;
    }

    namespace esp {
        inline float boxThickness = 1.5f;
        inline float snaplineWidth = 1.0f;

        inline ImVec4 boxColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
        inline ImVec4 snaplineColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
        inline ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White

        inline float fontSize = 15.0f;
    }
}
