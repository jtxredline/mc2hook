#include "keyboard.h"

declfield(ioKeyboard::m_Active)(0x85F888);
declfield(ioKeyboard::m_Keys)(0x85F898);
declfield(ioKeyboard::g_keybDev)(0x85F890);
declfield(ioKeyboard::sm_CurrentState)(0x85F998);

declfield(ioKeyboard::byte_85F8C2)(0x85F8C2);
declfield(ioKeyboard::byte_85F8CE)(0x85F8CE);

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

int ioKeyboard::ScancodesToAscii(char* out)
{
    bool shiftState = GetKey(DIK_LSHIFT) || GetKey(DIK_RSHIFT); //byte_85F8C2[0x100 * m_Active] || byte_85F8CE[0x100 * m_Active];
    bool altState = GetKey(DIK_LALT) || GetKey(DIK_RALT);

    int idx = 0;
    int numScancodes = ioKeyboard::GetBufferedInput(out, 3);
    static bool useAzerty = HookConfig::GetInt("Input", "UseAzerty", 0);

    for (int i = 0; i < numScancodes; ++i)
    {
        char& ascii = out[idx];
        char scancode = *(i + out);

        if (useAzerty)
        {
            switch (scancode)
            {
            case DIK_A: scancode = DIK_Q; break;
            case DIK_Q: scancode = DIK_A; break;
            case DIK_Z: scancode = DIK_W; break;
            case DIK_W: scancode = DIK_Z; break;
            //case DIK_COMMA: scancode = DIK_M; break;
            //case DIK_M: scancode = DIK_COMMA; break;
            }
        }

        switch (scancode)
        {
        case DIK_A:          ascii = shiftState ? 'A' : 'a'; break;
        case DIK_Z:          ascii = shiftState ? 'Z' : 'z'; break;
        case DIK_Q:          ascii = shiftState ? 'Q' : 'q'; break;
        case DIK_W:          ascii = shiftState ? 'W' : 'w'; break;
        case DIK_COMMA:      ascii = shiftState ? '<' : ','; break;
        case DIK_M:          ascii = shiftState ? 'M' : 'm'; break;

        case DIK_1: goto LABEL_8;
        case DIK_2: goto LABEL_10;
        case DIK_3: goto LABEL_12;
        case DIK_4: goto LABEL_14;
        case DIK_5: goto LABEL_16;
        case DIK_6: goto LABEL_18;
        case DIK_7: goto LABEL_20;
        case DIK_8: goto LABEL_22;
        case DIK_9: goto LABEL_24;
        case DIK_0: goto LABEL_26;

        case DIK_MINUS:      ascii = shiftState ? '_' : '-'; break;
        case DIK_EQUALS:     ascii = shiftState ? '+' : '='; break;
        case DIK_BACK:       ascii = 0x0E; break; // Backspace
        case DIK_E:          ascii = shiftState ? 'E' : 'e'; break;
        case DIK_R:          ascii = shiftState ? 'R' : 'r'; break;
        case DIK_T:          ascii = shiftState ? 'T' : 't'; break;
        case DIK_Y:          ascii = shiftState ? 'Y' : 'y'; break;
        case DIK_U:          ascii = shiftState ? 'U' : 'u'; break;
        case DIK_I:          ascii = shiftState ? 'I' : 'i'; break;
        case DIK_O:          ascii = shiftState ? 'O' : 'o'; break;
        case DIK_P:          ascii = shiftState ? 'P' : 'p'; break;
        case DIK_LBRACKET:   ascii = shiftState ? '{' : '['; break;
        case DIK_RBRACKET:   ascii = shiftState ? '}' : ']'; break;
        case DIK_S:          ascii = shiftState ? 'S' : 's'; break;
        case DIK_D:          ascii = shiftState ? 'D' : 'd'; break;
        case DIK_F:          ascii = shiftState ? 'F' : 'f'; break;
        case DIK_G:          ascii = shiftState ? 'G' : 'g'; break;
        case DIK_H:          ascii = shiftState ? 'H' : 'h'; break;
        case DIK_J:          ascii = shiftState ? 'J' : 'j'; break;
        case DIK_K:          ascii = shiftState ? 'K' : 'k'; break;
        case DIK_L:          ascii = shiftState ? 'L' : 'l'; break;
        case DIK_SEMICOLON:  ascii = shiftState ? ':' : ';'; break;
        case DIK_APOSTROPHE: ascii = shiftState ? '"' : '\''; break;
        case DIK_BACKSLASH:  ascii = shiftState ? '|' : '\\'; break;
        case DIK_X:          ascii = shiftState ? 'X' : 'x'; break;
        case DIK_C:          ascii = shiftState ? 'C' : 'c'; break;
        case DIK_V:          ascii = shiftState ? 'V' : 'v'; break;
        case DIK_B:          ascii = shiftState ? 'B' : 'b'; break;
        case DIK_N:          ascii = shiftState ? 'N' : 'n'; break;
        case DIK_PERIOD:     ascii = shiftState ? '>' : '.'; break;
        case DIK_SLASH:      ascii = shiftState ? '?' : '/'; break;
        case DIK_SPACE:      ascii = ' '; break;

        case DIK_NUMPAD7:
            if (shiftState) continue;
        LABEL_20:
            ascii = shiftState ? '&' : '7'; break;
        case DIK_NUMPAD8:
            if (shiftState) continue;
        LABEL_22:
            ascii = shiftState ? '*' : '8'; break;
        case DIK_NUMPAD9:
            if (shiftState) continue;
        LABEL_24:
            ascii = shiftState ? '(' : '9'; break;
        case DIK_NUMPAD4:
            if (shiftState) continue;
        LABEL_14:
            ascii = shiftState ? '$' : '4'; break;
        case DIK_NUMPAD5:
            if (shiftState) continue;
        LABEL_16:
            ascii = shiftState ? '%' : '5'; break;
        case DIK_NUMPAD6:
            if (shiftState) continue;
        LABEL_18:
            ascii = shiftState ? '^' : '6'; break;
        case DIK_NUMPAD1:
            if (shiftState) continue;
        LABEL_8:
            ascii = shiftState ? '!' : '1'; break;
        case DIK_NUMPAD2:
            if (shiftState) continue;
        LABEL_10:
            ascii = shiftState ? '@' : '2'; break;
        case DIK_NUMPAD3:
            if (shiftState) continue;
        LABEL_12:
            ascii = shiftState ? '#' : '3'; break;
        case DIK_NUMPAD0:
            if (shiftState) continue;
        LABEL_26:
            ascii = shiftState ? ')' : '0'; break;
        default:
            continue;
        }
        ++idx;
    }

    return idx;
}

/*
int ioKeyboard::ScancodesToAscii(char* out)
{
    bool shiftState = byte_85F8C2[0x100 * m_Active] || byte_85F8CE[0x100 * m_Active];
    //bool shiftState = GetKey(DIK_LSHIFT) || GetKey(DIK_RSHIFT); //byte_85F8C2[0x100 * m_Active] || byte_85F8CE[0x100 * m_Active];
    //bool altState = GetKey(DIK_LALT) || GetKey(DIK_RALT);
    int idx = 0;
    int numScancodes = ioKeyboard::GetBufferedInput(out, 3);
    static bool useAzerty = HookConfig::GetInt("Input", "UseAzerty", 0);

    for (int i = 0; i < numScancodes; ++i)
    {
        char& ascii = out[idx];

        switch (*(i + out))
        {
            //case DIK_A:          ascii = shiftState ? 'A' : 'a'; break;
            //case DIK_Z:          ascii = shiftState ? 'Z' : 'z'; break;
            //case DIK_Q:          ascii = shiftState ? 'Q' : 'q'; break;
            //case DIK_W:          ascii = shiftState ? 'W' : 'w'; break;
            //case DIK_COMMA:      ascii = shiftState ? '<' : ','; break;
            //case DIK_M:          ascii = shiftState ? 'M' : 'm'; break;

        case DIK_A:
            if (!useAzerty) ascii = shiftState ? 'A' : 'a';
            else            ascii = shiftState ? 'Q' : 'q'; break;
        case DIK_Z:
            if (!useAzerty) ascii = shiftState ? 'Z' : 'z';
            else            ascii = shiftState ? 'W' : 'w'; break;
        case DIK_Q:
            if (!useAzerty) ascii = shiftState ? 'Q' : 'q';
            else            ascii = shiftState ? 'A' : 'a'; break;
        case DIK_W:
            if (!useAzerty) ascii = shiftState ? 'W' : 'w';
            else            ascii = shiftState ? 'Z' : 'z'; break;
        case DIK_COMMA:
            if (!useAzerty) ascii = shiftState ? '<' : ',';
            else            ascii = shiftState ? 'M' : 'm'; break;
        case DIK_M:
            if (!useAzerty) ascii = shiftState ? 'M' : 'm';
            else            ascii = shiftState ? '<' : ','; break;

        case DIK_1: goto LABEL_8;
        case DIK_2: goto LABEL_10;
        case DIK_3: goto LABEL_12;
        case DIK_4: goto LABEL_14;
        case DIK_5: goto LABEL_16;
        case DIK_6: goto LABEL_18;
        case DIK_7: goto LABEL_20;
        case DIK_8: goto LABEL_22;
        case DIK_9: goto LABEL_24;
        case DIK_0: goto LABEL_26;

        case DIK_MINUS:      ascii = shiftState ? '_' : '-'; break;
        case DIK_EQUALS:     ascii = shiftState ? '+' : '='; break;
        case DIK_BACK:       ascii = 0x0E; break; // Backspace
        case DIK_E:          ascii = shiftState ? 'E' : 'e'; break;
        case DIK_R:          ascii = shiftState ? 'R' : 'r'; break;
        case DIK_T:          ascii = shiftState ? 'T' : 't'; break;
        case DIK_Y:          ascii = shiftState ? 'Y' : 'y'; break;
        case DIK_U:          ascii = shiftState ? 'U' : 'u'; break;
        case DIK_I:          ascii = shiftState ? 'I' : 'i'; break;
        case DIK_O:          ascii = shiftState ? 'O' : 'o'; break;
        case DIK_P:          ascii = shiftState ? 'P' : 'p'; break;
        case DIK_LBRACKET:   ascii = shiftState ? '{' : '['; break;
        case DIK_RBRACKET:   ascii = shiftState ? '}' : ']'; break;
        case DIK_S:          ascii = shiftState ? 'S' : 's'; break;
        case DIK_D:          ascii = shiftState ? 'D' : 'd'; break;
        case DIK_F:          ascii = shiftState ? 'F' : 'f'; break;
        case DIK_G:          ascii = shiftState ? 'G' : 'g'; break;
        case DIK_H:          ascii = shiftState ? 'H' : 'h'; break;
        case DIK_J:          ascii = shiftState ? 'J' : 'j'; break;
        case DIK_K:          ascii = shiftState ? 'K' : 'k'; break;
        case DIK_L:          ascii = shiftState ? 'L' : 'l'; break;
        case DIK_SEMICOLON:  ascii = shiftState ? ':' : ';'; break;
        case DIK_APOSTROPHE: ascii = shiftState ? '"' : '\''; break;
        case DIK_BACKSLASH:  ascii = shiftState ? '|' : '\\'; break;
        case DIK_X:          ascii = shiftState ? 'X' : 'x'; break;
        case DIK_C:          ascii = shiftState ? 'C' : 'c'; break;
        case DIK_V:          ascii = shiftState ? 'V' : 'v'; break;
        case DIK_B:          ascii = shiftState ? 'B' : 'b'; break;
        case DIK_N:          ascii = shiftState ? 'N' : 'n'; break;
        case DIK_PERIOD:     ascii = shiftState ? '>' : '.'; break;
        case DIK_SLASH:      ascii = shiftState ? '?' : '/'; break;
        case DIK_SPACE:      ascii = ' '; break;

        case DIK_NUMPAD7:
            if (shiftState) continue;
        LABEL_20:
            ascii = shiftState ? '&' : '7'; break;
        case DIK_NUMPAD8:
            if (shiftState) continue;
        LABEL_22:
            ascii = shiftState ? '*' : '8'; break;
        case DIK_NUMPAD9:
            if (shiftState) continue;
        LABEL_24:
            ascii = shiftState ? '(' : '9'; break;
        case DIK_NUMPAD4:
            if (shiftState) continue;
        LABEL_14:
            ascii = shiftState ? '$' : '4'; break;
        case DIK_NUMPAD5:
            if (shiftState) continue;
        LABEL_16:
            ascii = shiftState ? '%' : '5'; break;
        case DIK_NUMPAD6:
            if (shiftState) continue;
        LABEL_18:
            ascii = shiftState ? '^' : '6'; break;
        case DIK_NUMPAD1:
            if (shiftState) continue;
        LABEL_8:
            ascii = shiftState ? '!' : '1'; break;
        case DIK_NUMPAD2:
            if (shiftState) continue;
        LABEL_10:
            ascii = shiftState ? '@' : '2'; break;
        case DIK_NUMPAD3:
            if (shiftState) continue;
        LABEL_12:
            ascii = shiftState ? '#' : '3'; break;
        case DIK_NUMPAD0:
            if (shiftState) continue;
        LABEL_26:
            ascii = shiftState ? ')' : '0'; break;
        default:
            continue;
        }
        ++idx;
    }

    return idx;
}
*/

int ioKeyboard::GetBufferedInput(char* dest, int maxLen)
{
    if (!g_keybDev)
        return 0;

    DIDEVICEOBJECTDATA events[32];
    DWORD numEvents = 32;

    if (g_keybDev->GetDeviceData(
        sizeof(DIDEVICEOBJECTDATA),
        events,
        &numEvents,
        0) < 0)
    {
        return 0;
    }

    int count = 0;

    if (numEvents > 32)
        numEvents = 32;

    for (DWORD i = 0; i < numEvents; ++i)
    {
        // key-down only
        if (!(events[i].dwData & 0x80))
            continue;

        uint8_t scanCode = (uint8_t)(events[i].dwOfs & 0xFF);

        if (!dest)
        {
            m_Keys.ptr()[KEYINDEX(m_Active.get(), scanCode)] = 0x80;
        }

        if (count < maxLen)
        {
            dest[count++] = scanCode;
        }
    }

    return count;
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
