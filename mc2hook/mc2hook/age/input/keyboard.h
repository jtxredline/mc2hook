#pragma once
#include <mc2hook\mc2hook.h>
#include <dinput.h>

class ioKeyboard
{
private:
    static hook::Type<int> m_Active;
    static hook::Type<unsigned char [512]> m_Keys;
    static hook::Type<IDirectInputDevice8*> g_keybDev;
    static hook::Type<char[256]> sm_CurrentState;

    static hook::Type<char[256]> byte_85F8C2;
    static hook::Type<char[256]> byte_85F8CE;

    static bool GetActiveFlag()
    {
        return m_Active != 0;
    }

public:
    static bool GetKey(uint8_t keycode);
    static bool GetKeyDown(uint8_t keycode);
    static bool GetKeyUp(uint8_t keycode);
    static int ScancodesToAscii(char* out);
    static int GetBufferedInput(char* dest, int maxLen);

    void Update();
};
