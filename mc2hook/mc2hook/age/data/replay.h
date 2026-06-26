#pragma once
#include <mc2hook\mc2hook.h>

class datMemStream;

class datReplay
{
public:
    static hook::Type<int> sm_FrameSize;
    static hook::Type<datMemStream*> sm_ReplayFile;
    static hook::Type<bool> sm_Playback;

public:
    static uint8_t GetByte();
    static int GetInt();
    static void RecordInt(int a1);
    static uint16_t ReadFrameUInt16();

    static bool BeginRecording(datMemStream* stream);
    static void Reset();
    static void datReplay_614770();
};
