#pragma once
#include <mc2hook\mc2hook.h>

struct InputMapEntry
{
    int type;
    int value;
};

class ioDeviceWrapper
{
public:
    void* unk0;
    int* device;
    uint8_t* inputData;

public:
    float GetInput(int axisIdx);
    bool sub_467E90(int a2);
    void sub_467DF0(); // UpdateInputDevices?
    bool sub_467FA0();

    static hook::Type<InputMapEntry*> g_InputMap;
};
