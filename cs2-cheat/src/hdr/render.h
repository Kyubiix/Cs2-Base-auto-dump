#pragma once

#include "vector.h"
#include "globals.h"
#include "../memory/memory.h"

#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_dx11.h"
#include "../ext/imgui/imgui_impl_win32.h"

void RenderUI(Memory& mem) {
    // Start a new ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // If the menu is toggled, render the menu
    if (toggles::menu) {
        // Adjust window style to make the menu interactable
        LONG_PTR windowStyle = GetWindowLongPtr(globals::hwnd, GWL_EXSTYLE);
        windowStyle &= ~WS_EX_TRANSPARENT; // Disable "click-through" mode
        SetWindowLongPtr(globals::hwnd, GWL_EXSTYLE, windowStyle);

        // Render the main menu window
        ImGui::SetNextWindowSize(ImVec2(500, 500));
        ImGui::Begin("ESP Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // ESP Feature Toggles
        ImGui::Checkbox("Enable ESP", &toggles::hacks::bESP);
        ImGui::Checkbox("Snaplines", &toggles::hacks::bSnaplines);
        ImGui::Checkbox("Health", &toggles::hacks::bHealth);
        ImGui::Checkbox("Names", &toggles::hacks::bNames);
        ImGui::Checkbox("Distance", &toggles::hacks::bDistance);

        // Customization Sliders
        ImGui::SliderFloat("Box Thickness", &toggles::esp::boxThickness, 0.1f, 5.0f, "%.1f");
        ImGui::SliderFloat("Snapline Width", &toggles::esp::snaplineWidth, 0.1f, 5.0f, "%.1f");

        // Color Pickers
        ImGui::ColorEdit4("Box Color", reinterpret_cast<float*>(&toggles::esp::boxColor));
        ImGui::ColorEdit4("Snapline Color", reinterpret_cast<float*>(&toggles::esp::snaplineColor));
        ImGui::ColorEdit4("Text Color", reinterpret_cast<float*>(&toggles::esp::textColor));

        // Font Size
        ImGui::SliderFloat("Font Size", &toggles::esp::fontSize, 10.0f, 30.0f, "%.1f");

        ImGui::End(); // End the menu window
    }
    else {
        // If the menu is not open, re-enable transparent window style for overlay
        LONG_PTR windowStyle = GetWindowLongPtr(globals::hwnd, GWL_EXSTYLE);
        windowStyle |= WS_EX_TRANSPARENT;
        SetWindowLongPtr(globals::hwnd, GWL_EXSTYLE, windowStyle);
    }

    // If ESP features are toggled, call the ESP loop
    if (toggles::hacks::bESP || toggles::hacks::bSnaplines || toggles::hacks::bHealth || toggles::hacks::bNames || toggles::hacks::bDistance) {
        hack::espLoop(mem);
    }

    // Render ImGui to the screen
    ImGui::Render();
}
