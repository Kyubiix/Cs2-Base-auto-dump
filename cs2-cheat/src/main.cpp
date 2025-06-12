#include <iostream>
#include <thread>
#include <d3d11.h>
#include <windows.h>
#include <DirectXColors.h>

#include "hacks/esp.h"
#include "hdr/globals.h"
#include "hdr/render.h"
#include "hdr/console.h"
#include "memory/memory.h"

#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_dx11.h"
#include "../ext/imgui/imgui_impl_win32.h"

const int windowWidth = 1920;
const int windowHeight = 1080;

IDXGISwapChain* swapChain = nullptr;
ID3D11Device* device = nullptr;
ID3D11DeviceContext* deviceContext = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool InitializeDirectX(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = windowWidth;
    scd.BufferDesc.Height = windowHeight;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    if (D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
        &scd, &swapChain, &device, nullptr, &deviceContext) != S_OK) {
        return false;
    }

    // Create a render target view
    ID3D11Texture2D* pBackBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    device->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
    pBackBuffer->Release();

    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CreateConsole();
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "External Overlay Class";
    RegisterClassEx(&wc);
    
    Memory mem{ "cs2.exe" };

    do {
        hack::base_module = mem.GetModuleAddress("client.dll");
        if (hack::base_module == 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } while (hack::base_module == 0);

    globals::hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        wc.lpszClassName, "", WS_POPUP,
        0, 0, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

    SetLayeredWindowAttributes(globals::hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    if (!InitializeDirectX(globals::hwnd))
        return -1;

    ImGui::CreateContext();
    ImGuiIO& io = ::ImGui::GetIO();
    io.IniFilename = NULL;
    ImGui_ImplWin32_Init(globals::hwnd);
    ImGui_ImplDX11_Init(device, deviceContext);
    bool running = true;

    ShowWindow(globals::hwnd, nCmdShow);
    UpdateWindow(globals::hwnd);

    MSG msg = {};

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            deviceContext->ClearRenderTargetView(renderTargetView, DirectX::Colors::Transparent);

            static bool hasToggled = false;

            if (GetAsyncKeyState(VK_INSERT) & 1 && !hasToggled) {
                toggles::menu = !toggles::menu;
                SetForegroundWindow(globals::hwnd);
                hasToggled = true;
            }
            else if (!GetAsyncKeyState(VK_INSERT) & 1) {
                LONG_PTR windowStyle = GetWindowLongPtr(globals::hwnd, GWL_EXSTYLE);
                SetWindowLongPtr(globals::hwnd, GWL_EXSTYLE, windowStyle | WS_EX_TRANSPARENT);
                hasToggled = false;
            }

            if (GetAsyncKeyState(VK_END) & 1) {
                return 0;
            }

            RenderUI(mem);

            deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            swapChain->Present(1, 0);
        }
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    renderTargetView->Release();
    swapChain->Release();
    deviceContext->Release();
    device->Release();

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            // window resizing stuff
        }
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}