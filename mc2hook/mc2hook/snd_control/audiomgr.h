#pragma once
#include <mc2hook/mc2hook.h>

class sndControlVolumeGroup;

class sndAudioManager
{
public:
    int dword_00;
    int dword_04;
    int dword_08;
    int dword_0c;
    int dword_10;
    int dword_14;
    int dword_18;
    int dword_1c;
    int dword_20;
    int dword_24;
    int dword_28;
    sndControlVolumeGroup* m_ControlVolumeGroup;

public:
    static hook::Type<sndAudioManager*> smInstance;
};
