#pragma once
#include <mc2hook\mc2hook.h>

class sndAudioManager {
public:
    void sub_52B3D0();
    void sub_52B3C0();

public:
    static hook::Type<sndAudioManager*> Instance;
};
