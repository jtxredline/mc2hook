#pragma once
#include <mc2hook\mc2hook.h>

class datReplay
{
public:
    static hook::Type<int> sm_FrameSize;
    static hook::Type<int> sm_ReplayFile;

public:
    static uint8_t GetByte();
    static int GetInt();
    static uint16_t ReadFrameUInt16();
};
