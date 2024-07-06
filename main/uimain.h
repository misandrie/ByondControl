#pragma once
#include <windows.h>
#include <d3d.h>
#include "dllmain.h"
#include "pipes.h"
#include "kiero.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "imgui_demo.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT ImGui_ImplWin32_WndProcHandlerMod(HWND hwnd, WPARAM wParam, LPARAM lParam, bool mouse);

static WNDPROC oWndProc = NULL;
static HHOOK mhook;
static HHOOK khook;
HWND hwnd;

LRESULT __stdcall WndProc(const HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProcA(oWndProc, hwnd, uMsg, wParam, lParam);
}

long __stdcall hkReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    long result = oReset(pDevice, pPresentationParameters);
    ImGui_ImplDX9_CreateDeviceObjects();

    return result;
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION)  // Nothing to do
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    ImGui_ImplWin32_WndProcHandlerMod(hwnd, wParam, lParam, true);
    return DefWindowProc(NULL, nCode, wParam, lParam);
}
LRESULT CALLBACK LowLevelKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION)  // Nothing to do
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    ImGui_ImplWin32_WndProcHandlerMod(hwnd, wParam, lParam, false);
    return DefWindowProc(NULL, nCode, wParam, lParam);
}
//void exiting() {
//    ctrl_handler(CTRL_CLOSE_EVENT);
//}
DWORD WINAPI HookMouse(HMODULE hModule) {
    //std::atexit(exiting); //possibly causes crashes. If so, remove this

    mhook = SetWindowsHookExA(WH_MOUSE_LL, MouseHookProc, nullptr, 0); //hook mouse
    khook = SetWindowsHookExA(WH_KEYBOARD_LL, LowLevelKeyboardHookProc, nullptr, 0); //hook keyboard
    GetMessage(nullptr, nullptr, 0, 0); //make the thread not close
    return TRUE;
}

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandlerMod(HWND hwnd, WPARAM wParam, LPARAM lParam, bool mouse) //if it's not a mouse callback it's a keyboard one
{
    if (ImGui::GetCurrentContext() == NULL)
        return 0;

    ImGuiIO& io = ImGui::GetIO();
    if (mouse) {
        switch (wParam)
        {
        case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
        {
            int button = 0;
            if (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONDBLCLK) { button = 0; }
            if (wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONDBLCLK) { button = 1; }
            if (wParam == WM_MBUTTONDOWN || wParam == WM_MBUTTONDBLCLK) { button = 2; }
            if (wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(reinterpret_cast<MSLLHOOKSTRUCT*>(lParam)->mouseData) == XBUTTON1) ? 3 : 4; }
            if (!ImGui::IsAnyMouseDown() && ::GetCapture() == NULL)
                ::SetCapture(hwnd);
            io.MouseDown[button] = true;
            FireKeyBinds(button, true, io.KeysDown[VK_LCONTROL], io.KeysDown[VK_LSHIFT], io.KeysDown[VK_LMENU]);
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
            int button = 0;
            if (wParam == WM_LBUTTONUP) { button = 0; }
            if (wParam == WM_RBUTTONUP) { button = 1; }
            if (wParam == WM_MBUTTONUP) { button = 2; }
            if (wParam == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(lParam) == XBUTTON1) ? 3 : 4; }
            io.MouseDown[button] = false;
            if (!ImGui::IsAnyMouseDown() && ::GetCapture() == hwnd)
                ::ReleaseCapture();
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(info->mouseData) / (float)WHEEL_DELTA;
            return 0;
        }
        case WM_MOUSEHWHEEL:
        {
            MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(info->mouseData) / (float)WHEEL_DELTA;
            return 0;
        }
        }
    }
    else {
        KBDLLHOOKSTRUCT* keyboardInfo = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        switch (wParam) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (keyboardInfo->vkCode < 512)
                io.KeysDown[keyboardInfo->vkCode] = true;
            GetKeyState(keyboardInfo->vkCode);
            //PIPELOG("%u %u %u", keyboardInfo->vkCode, VK_MENU, VK_LMENU);
            BYTE keys[256];
            if (GetKeyboardState(keys)) {
                FireKeyBinds(keyboardInfo->vkCode, false, io.KeysDown[VK_LCONTROL], io.KeysDown[VK_LSHIFT], io.KeysDown[VK_LMENU]);
                //PIPELOG("%u %u %u", io.KeysDown[VK_LCONTROL], io.KeysDown[VK_LSHIFT], io.KeysDown[VK_LMENU]);
                WCHAR keyPressed[10];
                HKL keyboardLayout = GetKeyboardLayout(NULL);
                // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
                if (ToUnicodeEx(keyboardInfo->vkCode, keyboardInfo->scanCode, (BYTE*)keys, keyPressed, 10, 2, keyboardLayout))
                {
                    //wprintf(L"key: %ws scanCode: %d vkCode: %d\n lShift: %x rShift: %x\n", keyPressed, keyboardInfo->scanCode, keyboardInfo->vkCode, (BYTE)io.KeysDown[VK_LSHIFT], (BYTE)io.KeysDown[VK_RSHIFT]);
                    for (int i = 0; i < 10; i++)
                    {
                        if (keyPressed[i] > 0 && keyPressed[i] < 0x10000)
                            io.AddInputCharacterUTF16((unsigned short)keyPressed[i]);
                    }
                }
            }
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (keyboardInfo->vkCode < 512)
                io.KeysDown[keyboardInfo->vkCode] = 0;
            //PIPELOG("%u %u %u", keyboardInfo->vkCode, VK_MENU, VK_LMENU);
            break;
        }
        return 0;
    }
    return 0;
}

long __stdcall hkPresent(LPDIRECT3DDEVICE9 pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
    static bool init = true;

    if (init)
    {
        init = false;
        D3DDEVICE_CREATION_PARAMETERS params;
        pDevice->GetCreationParameters(&params);

        oWndProc = (WNDPROC)::SetWindowLongPtr(params.hFocusWindow, GWLP_WNDPROC, (LONG)WndProc);
        hwnd = params.hFocusWindow;
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)HookMouse, NULL, 0, NULL);

        ImGui::CreateContext();

        ImGuiIO& imio = ImGui::GetIO();
        imio.IniFilename = NULL;

        ImGui_ImplWin32_Init(params.hFocusWindow);
        ImGui_ImplDX9_Init(pDevice);

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 5.3f;
        style.FrameRounding = 0;
        style.ScrollbarRounding = 0;
    }

    pDevice->BeginScene();

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGuiIO& imio = ImGui::GetIO();

    if (mainWindow != nullptr) {
        int graphicsWind = (int)mainWindow + 260;
        imio.MousePos.x -= (float)(*(int*)(graphicsWind + 0xd0));
        imio.MousePos.y -= (float)(*(int*)(graphicsWind + 0xd4));
    }
    ImGui::NewFrame();

    RenderImGui();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    pDevice->EndScene();

    return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

DWORD WINAPI UIThread()
{
load:
    MessageBoxA(0, "click OK once fully loaded and in fullscreen", "load", MB_OK);

    kiero::Status::Enum result = kiero::init(kiero::RenderType::D3D9);
    if (result == kiero::Status::Success)
    {
        PIPELOG("binding ui");
        kiero::bind(16, (void**)&oReset, hkReset);
        kiero::bind(17, (void**)&oPresent, hkPresent);
    }
    else if (result != kiero::Status::AlreadyInitializedError)
    {
        char buff[50];
        sprintf_s(buff, "kiero failed to init %d", result);
        MessageBoxA(0, buff, "error", MB_OK);
        goto load;
    }

    return TRUE;
}