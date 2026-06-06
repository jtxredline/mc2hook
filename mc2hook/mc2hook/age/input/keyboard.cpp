#include "keyboard.h"

declfield(ioKeyboard::m_Active)(0x85F888);
declfield(ioKeyboard::m_Keys)(0x85F898);
declfield(ioKeyboard::g_keybDev)(0x85F890);
declfield(ioKeyboard::sm_CurrentState)(0x85F998);

#define KEYINDEX(active, index) ((256 * active) + index)

bool ioKeyboard::GetKey(uint8_t keycode)
{
    return m_Keys.ptr()[KEYINDEX(m_Active.get(), keycode)];
}

bool ioKeyboard::GetKeyDown(uint8_t keycode)
{
    return (m_Keys.ptr()[KEYINDEX(0, keycode)] ^ m_Keys.ptr()[KEYINDEX(1, keycode)])
        & m_Keys.ptr()[KEYINDEX(GetActiveFlag(), keycode)];
}

bool ioKeyboard::GetKeyUp(uint8_t keycode)
{
    return (m_Keys.ptr()[KEYINDEX(0, keycode)] ^ m_Keys.ptr()[KEYINDEX(1, keycode)])
            & m_Keys.ptr()[KEYINDEX(!GetActiveFlag(), keycode)];
}

void ioKeyboard::Update()
{
    if (g_keybDev)
    {
        unsigned char* buffer = &m_Keys[256 * (m_Active ^ 1)];

        // Try GetDeviceState - if it fails, re-acquire and retry once
        // DirectInput returns DI_OK (0) on success
        bool stateRead = g_keybDev->GetDeviceState(256, buffer) == DI_OK;

        if (!stateRead)
        {
            stateRead =
                g_keybDev->Acquire() == DI_OK &&
                g_keybDev->GetDeviceState(256, buffer) == DI_OK;
        }

        if (stateRead)
        {
            m_Active = m_Active ^ 1;
        }
    }
    else
    {
        // No DirectInput device - copy keys to current state buffer
        memcpy(sm_CurrentState, m_Keys, 0x100u);
    }
}
