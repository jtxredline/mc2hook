#include "pipeline.h"

#include <windows.h>
#include <dbt.h>
#include <initguid.h>
#include <hidclass.h>

#include <age/input/joystick.h>

declfield(gfxPipeline::m_iWidth) = 0x674FAC;
declfield(gfxPipeline::m_iHeight) = 0x674FB0;
declfield(gfxPipeline::m_X) = 0x85839C;
declfield(gfxPipeline::m_Y) = 0x858388;

declfield(gfxPipeline::hwndMain) = 0x858364;

/*declfield(gfxPipeline::windowTitle) = 0x858390;
declfield(gfxPipeline::ATOM_class) = 0x8583D4;
declfield(gfxPipeline::iconId) = 0x85837C;
declfield(gfxPipeline::menuResourceId) = 0x8583AC;
declfield(gfxPipeline::inWindow) = 0x858370;
declfield(gfxPipeline::hwndParent) = 0x858360;
declfield(gfxPipeline::bWinBorder) = 0x674FA8;
declfield(gfxPipeline::isMaximized) = 0x858371;
declfield(gfxPipeline::hMenu) = 0x858380;
declfield(gfxPipeline::windowStyleCache) = 0x8583A4;
declfield(gfxPipeline::windowRect) = 0x8583C4;
declfield(gfxPipeline::clientRect) = 0x8583B4;*/

void gfxPipeline::gfxWindowCreate(LPCSTR lpWindowName)
{
	hook::StaticThunk<0x5ED560>::Call<void>(lpWindowName); // Call original

    // This registers for device notifications (controller reconnected etc.)
    DEV_BROADCAST_DEVICEINTERFACE_A filter = {};
    filter.dbcc_size = sizeof(filter);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid = GUID_DEVINTERFACE_HID;

    HDEVNOTIFY notifyHandle =
        RegisterDeviceNotificationA(
            hwndMain,
            &filter,
            DEVICE_NOTIFY_WINDOW_HANDLE
        );
}

LRESULT gfxPipeline::InputWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Re-initialize reconnected controller
    if (msg == WM_DEVICECHANGE)
    {
        switch (wParam)
        {
        //case DBT_DEVICEREMOVECOMPLETE: // ioJoystick::EndAll()?
        case DBT_DEVICEARRIVAL:
            ioJoystick::BeginAll();
            break;
        }
    }

    return hook::StaticThunk<0x605830>::Call<LRESULT>(hWnd, msg, wParam, lParam); // Call original
}
