#pragma once

#include <iostream>
#include <thread>
#include "../memory/memory.h"
#include "../hdr/globals.h"
#include "../hdr/vector.h"

#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_impl_dx11.h"
#include "../../ext/imgui/imgui_impl_win32.h"

namespace hack {
    uintptr_t base_module = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));

    bool ValidateOffsets() {
        if (!offsets::dwEntityList || !offsets::dwViewMatrix || !offsets::dwLocalPlayer) {
            std::cerr << "Error: One or more offsets are not initialized!" << std::endl;
            return false;
        }
        return true;
    }

    bool ValidateBaseModule() {
        if (!base_module) {
            std::cerr << "Error: Base module (client.dll) not found!" << std::endl;
            return false;
        }
        return true;
    }

    void espLoop(Memory& mem) {
        if (!ValidateBaseModule() || !ValidateOffsets()) return;

        if (!mem.IsValid()) {
            std::cerr << "Error: Memory class is not valid!" << std::endl;
            return;
        }

        // Get the local player
        uintptr_t localPlayer = mem.Read<uintptr_t>(base_module + offsets::dwLocalPlayer);
        if (!localPlayer) {
            std::cerr << "Error: Local player not found!" << std::endl;
            return;
        }

        int localTeam = mem.Read<int>(localPlayer + offsets::m_iTeamNum);

        // Read view matrix
        view_matrix_t view_matrix = mem.Read<view_matrix_t>(base_module + offsets::dwViewMatrix);

        // Read entity list
        uintptr_t entity_list = mem.Read<uintptr_t>(base_module + offsets::dwEntityList);
        if (!entity_list) {
            std::cerr << "Error: Entity list not found!" << std::endl;
            return;
        }


        for (int i = 1; i < 32; i++) {
            uintptr_t list_entry = mem.Read<uintptr_t>(entity_list + (8 * (i & 0x7FFF) >> 9) + 16);
            if (!list_entry) continue;

            uintptr_t player = mem.Read<uintptr_t>(list_entry + 120 * (i & 0x1FF));
            if (!player) continue;

            // Validate player health
            int playerHealth = mem.Read<int>(player + offsets::dwPawnHealth);
            if (playerHealth <= 0 || playerHealth > 100) continue;

            std::uint32_t playerpawn = mem.Read<std::uint32_t>(player + offsets::dwPlayerPawn);
            uintptr_t list_entry2 = mem.Read<uintptr_t>(entity_list + 0x8 * ((playerpawn & 0x7FFF) >> 9) + 16);
            if (!list_entry2) continue;

            uintptr_t pCSPlayerPawn = mem.Read<uintptr_t>(list_entry2 + 120 * (playerpawn & 0x1FF));
            if (!pCSPlayerPawn) continue;

            Vector3 origin = mem.Read<Vector3>(pCSPlayerPawn + offsets::m_vecOrigin);
            if (pCSPlayerPawn == localPlayer) continue;

            // Get player team
            int playerTeam = mem.Read<int>(player + offsets::m_iTeamNum);

            Vector3 head = { origin.x, origin.y, origin.z + 75.f };

            Vector3 screenpos = origin.w2s(view_matrix);
            Vector3 screenhead = head.w2s(view_matrix);

            if (screenpos.z >= 0.01f) {
                if (localTeam == playerTeam) continue;

                float height = screenpos.y - screenhead.y;
                float width = height / 2.4f;

                // Draw ESP boxes
                if (toggles::hacks::bESP) {
                    ImGui::GetBackgroundDrawList()->AddRect(
                        { screenhead.x - width, screenhead.y },
                        { screenhead.x + width, screenpos.y },
                        ImColor(255, 255, 255),
                        1.f
                    );
                }

                // Draw snaplines
                if (toggles::hacks::bSnaplines) {
                    ImGui::GetBackgroundDrawList()->AddLine(
                        { screenpos.x, screenpos.y },
                        { 1920 / 2, 1080 },
                        ImColor(255, 255, 255)
                    );
                }
            }
        }
    }
}
