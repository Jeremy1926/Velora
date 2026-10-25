#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct IDXGISwapChain;

class FGUI
{
public:
    static void Run();

private:
    static LRESULT WINAPI WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

    static bool CreateDevice(HWND Window);

    static void DestroyDevice();

    static void CreateRenderTarget();

    static void DestroyRenderTarget();

    static void SetupStyle(float Scale);

    static void Draw();

    static ID3D11Device* Device;
    static ID3D11DeviceContext* DeviceContext;
    static IDXGISwapChain* SwapChain;
    static ID3D11RenderTargetView* RenderTargetView;
    static UINT ResizeWidth;
    static UINT ResizeHeight;
};
