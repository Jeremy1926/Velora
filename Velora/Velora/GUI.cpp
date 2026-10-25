#include "pch.h"
#include "GUI.h"
#include <d3d11.h>
#include <dwmapi.h>
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "ThirdParty/Ubuntu/Ubuntu.h"
#include "Logging/LogMacros.h"
#include "Config.h"
#include "Scanner.h"

#pragma comment(lib, "d3d11")

extern "C" IMAGE_DOS_HEADER __ImageBase;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ID3D11Device* FGUI::Device = nullptr;
ID3D11DeviceContext* FGUI::DeviceContext = nullptr;
IDXGISwapChain* FGUI::SwapChain = nullptr;
ID3D11RenderTargetView* FGUI::RenderTargetView = nullptr;
UINT FGUI::ResizeWidth = 0;
UINT FGUI::ResizeHeight = 0;
SRWLOCK FGUI::TitleLock = SRWLOCK_INIT;
TCHAR FGUI::Title[128] = TEXT("Velora");
bool FGUI::bTitleChanged = true;
bool FGUI::bStartBus = false;

void FGUI::Run()
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    const float Scale = ImGui_ImplWin32_GetDpiScaleForMonitor(MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    WNDCLASSEXW WindowClass = { sizeof(WindowClass), CS_CLASSDC, &FGUI::WindowProc, 0, 0, (HINSTANCE)&__ImageBase, nullptr, nullptr, nullptr, nullptr, TEXT("Velora"), nullptr };
    RegisterClassExW(&WindowClass);

    auto Window = CreateWindowExW(0, WindowClass.lpszClassName, TEXT("Velora"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, (int)(600 * Scale), (int)(450 * Scale), nullptr, nullptr, WindowClass.hInstance, nullptr);

    BOOL bDarkMode = TRUE;
    DwmSetWindowAttribute(Window, DWMWA_USE_IMMERSIVE_DARK_MODE, &bDarkMode, sizeof(bDarkMode));

    if (!CreateDevice(Window))
    {
        UE_LOG(LogVelora, Error, TEXT("Could not create the GUI device"));

        DestroyDevice();
        DestroyWindow(Window);
        UnregisterClassW(WindowClass.lpszClassName, WindowClass.hInstance);

        return;
    }

    ShowWindow(Window, SW_SHOWDEFAULT);
    UpdateWindow(Window);

    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;

    SetupStyle(Scale);

    ImGui_ImplWin32_Init(Window);
    ImGui_ImplDX11_Init(Device, DeviceContext);

    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(Ubuntu_compressed_data_base85, 14.f);

    bool bSwapChainOccluded = false;

    for (;;)
    {
        bool bQuit = false;
        MSG Message;

        while (PeekMessageW(&Message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageW(&Message);

            if (Message.message == WM_QUIT)
                bQuit = true;
        }

        if (bQuit)
            break;

        AcquireSRWLockExclusive(&TitleLock);

        if (bTitleChanged)
        {
            SetWindowTextW(Window, Title);
            bTitleChanged = false;
        }

        ReleaseSRWLockExclusive(&TitleLock);

        if (bSwapChainOccluded && SwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            Sleep(10);
            continue;
        }

        bSwapChainOccluded = false;

        if (ResizeWidth && ResizeHeight)
        {
            DestroyRenderTarget();
            SwapChain->ResizeBuffers(0, ResizeWidth, ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            ResizeWidth = ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        Draw();

        ImGui::Render();

        DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);
        DeviceContext->ClearRenderTargetView(RenderTargetView, &ImGui::GetStyle().Colors[ImGuiCol_WindowBg].x);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        bSwapChainOccluded = SwapChain->Present(1, 0) == DXGI_STATUS_OCCLUDED;
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DestroyDevice();
    DestroyWindow(Window);
    UnregisterClassW(WindowClass.lpszClassName, WindowClass.hInstance);
}

void FGUI::SetTitle(const TCHAR* InTitle)
{
    AcquireSRWLockExclusive(&TitleLock);

    wcscpy_s(Title, InTitle);
    bTitleChanged = true;

    ReleaseSRWLockExclusive(&TitleLock);
}

LRESULT WINAPI FGUI::WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    if (ImGui_ImplWin32_WndProcHandler(Window, Message, WParam, LParam))
        return TRUE;

    switch (Message)
    {
    case WM_SIZE:
        if (WParam != SIZE_MINIMIZED)
        {
            ResizeWidth = LOWORD(LParam);
            ResizeHeight = HIWORD(LParam);
        }

        return 0;
    case WM_SYSCOMMAND:
        if ((WParam & 0xFFF0) == SC_KEYMENU)
            return 0;

        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(Window, Message, WParam, LParam);
}

bool FGUI::CreateDevice(HWND Window)
{
    DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.OutputWindow = Window;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.Windowed = TRUE;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL FeatureLevel;

    auto Result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, FeatureLevels, 2, D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &Device, &FeatureLevel, &DeviceContext);

    if (Result == DXGI_ERROR_UNSUPPORTED)
        Result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, FeatureLevels, 2, D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &Device, &FeatureLevel, &DeviceContext);

    if (Result != S_OK)
        return false;

    CreateRenderTarget();

    return true;
}

void FGUI::DestroyDevice()
{
    DestroyRenderTarget();

    if (SwapChain)
    {
        SwapChain->Release();
        SwapChain = nullptr;
    }

    if (DeviceContext)
    {
        DeviceContext->Release();
        DeviceContext = nullptr;
    }

    if (Device)
    {
        Device->Release();
        Device = nullptr;
    }
}

void FGUI::CreateRenderTarget()
{
    ID3D11Texture2D* BackBuffer;
    SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
    Device->CreateRenderTargetView(BackBuffer, nullptr, &RenderTargetView);
    BackBuffer->Release();
}

void FGUI::DestroyRenderTarget()
{
    if (RenderTargetView)
    {
        RenderTargetView->Release();
        RenderTargetView = nullptr;
    }
}

void FGUI::SetupStyle(float Scale)
{
    auto& Style = ImGui::GetStyle();

    Style.WindowPadding = ImVec2(8.f, 8.f);
    Style.FramePadding = ImVec2(5.f, 4.f);
    Style.ItemSpacing = ImVec2(8.f, 4.f);
    Style.ItemInnerSpacing = ImVec2(6.f, 4.f);
    Style.GrabMinSize = 10.f;
    Style.WindowBorderSize = 0.f;
    Style.WindowRounding = 0.f;
    Style.FrameRounding = 2.f;
    Style.GrabRounding = 2.f;
    Style.ScrollbarRounding = 2.f;
    Style.TabRounding = 4.f;
    Style.TabBarOverlineSize = 0.f;
    Style.ScaleAllSizes(Scale);

    Style.FontSizeBase = 14.f;
    Style.FontScaleDpi = Scale;

    auto Colors = Style.Colors;

    Colors[ImGuiCol_Text] = ImColor(220, 220, 220);
    Colors[ImGuiCol_TextDisabled] = ImColor(138, 138, 138);
    Colors[ImGuiCol_WindowBg] = ImColor(20, 20, 20);
    Colors[ImGuiCol_ChildBg] = ImColor(20, 20, 20);
    Colors[ImGuiCol_PopupBg] = ImColor(26, 26, 26);
    Colors[ImGuiCol_Border] = ImColor(42, 42, 42);
    Colors[ImGuiCol_FrameBg] = ImColor(38, 38, 38);
    Colors[ImGuiCol_FrameBgHovered] = ImColor(46, 46, 46);
    Colors[ImGuiCol_FrameBgActive] = ImColor(54, 54, 54);
    Colors[ImGuiCol_ScrollbarBg] = ImColor(20, 20, 20);
    Colors[ImGuiCol_ScrollbarGrab] = ImColor(51, 51, 51);
    Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(61, 61, 61);
    Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(71, 71, 71);
    Colors[ImGuiCol_CheckMark] = ImColor(232, 232, 232);
    Colors[ImGuiCol_CheckboxSelectedBg] = ImColor(38, 38, 38);
    Colors[ImGuiCol_InputTextCursor] = ImColor(220, 220, 220);
    Colors[ImGuiCol_SliderGrab] = ImColor(92, 92, 92);
    Colors[ImGuiCol_SliderGrabActive] = ImColor(112, 112, 112);
    Colors[ImGuiCol_Button] = ImColor(51, 51, 51);
    Colors[ImGuiCol_ButtonHovered] = ImColor(61, 61, 61);
    Colors[ImGuiCol_ButtonActive] = ImColor(71, 71, 71);
    Colors[ImGuiCol_Header] = ImColor(38, 38, 38);
    Colors[ImGuiCol_HeaderHovered] = ImColor(46, 46, 46);
    Colors[ImGuiCol_HeaderActive] = ImColor(54, 54, 54);
    Colors[ImGuiCol_Separator] = ImColor(42, 42, 42);
    Colors[ImGuiCol_SeparatorHovered] = ImColor(61, 61, 61);
    Colors[ImGuiCol_SeparatorActive] = ImColor(71, 71, 71);
    Colors[ImGuiCol_ResizeGrip] = ImColor(51, 51, 51);
    Colors[ImGuiCol_ResizeGripHovered] = ImColor(61, 61, 61);
    Colors[ImGuiCol_ResizeGripActive] = ImColor(71, 71, 71);
    Colors[ImGuiCol_Tab] = ImColor(30, 30, 30);
    Colors[ImGuiCol_TabHovered] = ImColor(61, 61, 61);
    Colors[ImGuiCol_TabSelected] = ImColor(51, 51, 51);
    Colors[ImGuiCol_TabDimmed] = ImColor(30, 30, 30);
    Colors[ImGuiCol_TabDimmedSelected] = ImColor(51, 51, 51);
    Colors[ImGuiCol_TextSelectedBg] = ImColor(71, 71, 71);
    Colors[ImGuiCol_NavCursor] = ImColor(112, 112, 112);
}

void FGUI::Draw()
{
    const auto Viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(Viewport->WorkPos);
    ImGui::SetNextWindowSize(Viewport->WorkSize);

    if (ImGui::Begin("Velora", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        if (ImGui::BeginTabBar("Tabs"))
        {
            if (ImGui::BeginTabItem("Main"))
            {
                ImGui::Dummy(ImVec2(0.f, ImGui::GetStyle().ItemSpacing.y * 2.f));

                ImGui::Checkbox("Lategame", &Config::bLategame);
                ImGui::Checkbox("Infinite Ammo", &Config::bInfiniteAmmo);
                ImGui::Checkbox("Infinite Materials", &Config::bInfiniteMaterials);

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        const auto& Style = ImGui::GetStyle();
        static char Command[256] = {};

        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - Style.WindowPadding.y - ImGui::GetFrameHeight() - Style.ItemSpacing.y - Style.SeparatorSize);
        ImGui::Separator();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Execute").x - ImGui::CalcTextSize("Start Bus").x - Style.FramePadding.x * 4.f - Style.ItemSpacing.x * 2.f);
        ImGui::InputTextWithHint("##Command", "Console command", Command, sizeof(Command));
        ImGui::SameLine();
        ImGui::Button("Execute");
        ImGui::SameLine();

        if (ImGui::Button("Start Bus"))
            bStartBus = true;
    }

    ImGui::End();
}
