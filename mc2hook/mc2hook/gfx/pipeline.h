#pragma once
#include <mc2hook\mc2hook.h>

class gfxPipeline
{
public:
    static hook::TypeProxy<int> m_iWidth;
    static hook::TypeProxy<int> m_iHeight;
    static hook::Type<int> m_X;
    static hook::Type<int> m_Y;

    static hook::Type<HWND> hwndMain;
    static hook::Type<int> dword_858384;

    static hook::Type<LPCSTR> windowTitle;
    static hook::Type<ATOM> ATOM_class;
    static hook::Type<LPCSTR> iconId;
    static hook::Type<unsigned short> menuResourceId;
    static hook::Type<bool> inWindow;
    static hook::Type<HWND> hwndParent;
    static hook::Type<bool> bWinBorder;
    static hook::Type<bool> isMaximized;
    static hook::Type<HMENU> hMenu;
    static hook::Type<LONG> windowStyleCache;
    static hook::Type<RECT> windowRect;
    static hook::Type<RECT> clientRect; 

    static hook::Func<WNDPROC> $gfxPipeline_gfxWindowProc;

public:
    static void gfxWindowCreate(LPCSTR windowName);
    static LRESULT APIENTRY gfxWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT InputWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void EndFrame();
};
