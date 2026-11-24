#pragma once
#include <mc2hook\mc2hook.h>

class CityModelLimitHandler
{
public:
    static void SetCPVSBufferLimit(uint32_t b);
    static void Install();
};
