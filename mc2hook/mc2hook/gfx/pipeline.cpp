#include "pipeline.h"

#include <windows.h>
#include <dbt.h>
#include <initguid.h>
#include <hidclass.h>

#include <input/joystick.h>
#include <data/args.h>

declfield(gfxPipeline::m_iWidth) = 0x674FAC;         // int
declfield(gfxPipeline::m_iHeight) = 0x674FB0;        // int
declfield(gfxPipeline::m_X) = 0x85839C;              // int
declfield(gfxPipeline::m_Y) = 0x858388;              // int

declfield(gfxPipeline::hwndMain) = 0x858364;         // HWND
declfield(gfxPipeline::dword_858384) = 0x858384;     // int

declfield(gfxPipeline::windowTitle) = 0x858390;      // LPCSTR
declfield(gfxPipeline::ATOM_class) = 0x8583D4;       // ATOM
declfield(gfxPipeline::iconId) = 0x85837C;           // LPCSTR
declfield(gfxPipeline::menuResourceId) = 0x8583AC;   // unsigned short
declfield(gfxPipeline::inWindow) = 0x858370;         // bool
declfield(gfxPipeline::hwndParent) = 0x858360;       // HWND
declfield(gfxPipeline::bWinBorder) = 0x674FA8;       // bool
declfield(gfxPipeline::isMaximized) = 0x858371;      // bool
declfield(gfxPipeline::hMenu) = 0x858380;            // HMENU
declfield(gfxPipeline::windowStyleCache) = 0x8583A4; // LONG
declfield(gfxPipeline::windowRect) = 0x8583C4;       // RECT
declfield(gfxPipeline::clientRect) = 0x8583B4;       // RECT

declfield(gfxPipeline::$gfxPipeline_gfxWindowProc) = 0x5ED4D0;


void gfxPipeline::gfxWindowCreate(LPCSTR lpWindowName)
{
    //hook::StaticThunk<0x5ED560>::Call<void>(lpWindowName); // Call original
    //return;

    LPCSTR className = "gfxWindow";

    if (windowTitle) lpWindowName = windowTitle;

    if (hwndMain) return;

    if (!ATOM_class)
    {
        WNDCLASSA wndClass = {};

        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = gfxPipeline::gfxWindowProc;

        if (iconId)
        {
            HMODULE module = GetModuleHandleA(nullptr);
            wndClass.hIcon = LoadIconA(module, iconId);
        }
        else
        {
            wndClass.hIcon = LoadIconA(nullptr, MAKEINTRESOURCEA(0x7F00));
        }

        wndClass.hCursor = LoadCursorA(nullptr, MAKEINTRESOURCEA(0x7F00));
        wndClass.hbrBackground = CreateSolidBrush(0);
        wndClass.lpszMenuName = nullptr;
        wndClass.lpszClassName = className;

        ATOM_class = RegisterClassA(&wndClass);
    }

    HWND parent = hwndParent;
    if (!inWindow)
    parent = nullptr;

    HDC dc = GetDC(nullptr);

    int screenWidth = GetDeviceCaps(dc, HORZRES);
    int screenHeight = GetDeviceCaps(dc, VERTRES);

    ReleaseDC(nullptr, dc);

    DWORD windowStyle;

    if (inWindow)
    {
        if (parent)
        {
            windowStyle = WS_CHILD;
        }
        else
        {
            windowStyle = bWinBorder ? WS_OVERLAPPEDWINDOW : WS_POPUP;
        }

        if (isMaximized) windowStyle |= WS_MAXIMIZE;
    }
    else
    {
        windowStyle = 0x80080000;
    }

    // Original clears bit 19 before AdjustWindowRect/CreateWindowEx.
    windowStyle &= ~0x80000;

    RECT rect;
    SetRect(&rect, 0, 0, m_iWidth, m_iHeight);
    AdjustWindowRect(&rect, windowStyle, FALSE);

    if (m_X == -1) m_X = (screenWidth + rect.left - rect.right) >> 1;

    if (m_Y == -1) m_Y = (screenHeight + rect.top - rect.bottom) >> 1;

    hwndMain = CreateWindowExA(
        0,
        className,
        lpWindowName,
        windowStyle,
        m_X,
        m_Y,
        rect.right - rect.left,
        rect.bottom - rect.top,
        parent,
        hMenu,
        nullptr,
        nullptr);

    windowStyleCache = GetWindowLongA(hwndMain, GWL_STYLE);

    GetWindowRect(hwndMain, &windowRect);

    GetClientRect(hwndMain, &clientRect);

    ShowWindow(hwndMain, SW_SHOW);

    UpdateWindow(hwndMain);
    SetFocus(hwndMain);

    // This registers for device notifications (controller reconnected etc.)
    DEV_BROADCAST_DEVICEINTERFACE_A filter = {};
    filter.dbcc_size = sizeof(filter);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid = GUID_DEVINTERFACE_HID;

    HDEVNOTIFY notifyHandle = RegisterDeviceNotificationA(hwndMain, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
}

LRESULT APIENTRY gfxPipeline::gfxWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //switch (uMsg)
    //{
    //case WM_ACTIVATEAPP:
    //{
    //    if (!wParam && datArgParser::Get("nopause"))
    //        return 0;

    //    break;
    //}
    //}

    return $gfxPipeline_gfxWindowProc(hWnd, uMsg, wParam, lParam); // Call original
}

LRESULT gfxPipeline::InputWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Re-initialize reconnected controller
    if (uMsg == WM_DEVICECHANGE)
    {
        switch (wParam)
        {
        //case DBT_DEVICEREMOVECOMPLETE: // ioJoystick::EndAll()?
        case DBT_DEVICEARRIVAL:
            ioJoystick::BeginAll();
            break;
        }
    }

    return hook::StaticThunk<0x605830>::Call<LRESULT>(hWnd, uMsg, wParam, lParam); // Call original
}

void gfxPipeline::EndFrame()
{
    hook::StaticThunk<0x5F0F00>::Call<void>(); // Call original
}
