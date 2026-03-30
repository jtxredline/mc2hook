#pragma once
#include <mc2hook\mc2hook.h>

class RenderDistanceHandler
{
public:
    float GetFloatHook();
    static void Install();
};
