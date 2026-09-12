#pragma once
#include <mc2hook\mc2hook.h>

class ioJoystick
{
public:
    static hook::Type<int> MaybeActiveJoystick;
    static hook::Type<int> sm_Sticks;
    static hook::Type<int> dword_85FC90;

public:
    static void BeginAll();
    static void Poll();
    static void Update();
    static void UpdateAll();
    static void End();
    static void EndAll();
};
