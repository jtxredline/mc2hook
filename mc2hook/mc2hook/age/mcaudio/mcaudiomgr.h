#pragma once
#include <mc2hook\mc2hook.h>

class mcAudioManager {
public:
    void sub_52B3D0();
    void sub_52B3C0();

public:
    static hook::Type<mcAudioManager*> Instance;
};
