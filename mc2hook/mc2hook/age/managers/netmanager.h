#pragma once
#include <mc2hook\mc2hook.h>

class mcNetManager
{
public:
    static hook::Type<bool> IsNetworkMode;
    static hook::Type<int> LocalPlayerID; // Part of mcPlayerManager?
};
