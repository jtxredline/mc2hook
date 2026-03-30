#pragma once
#include <mc2hook\mc2hook.h>

class ioJoystick
{
public:
    static hook::Type<int> MaybeActiveJoystick;
    static hook::Type<int> sm_Sticks;
    static hook::Type<int> dword_85FC90;
};
