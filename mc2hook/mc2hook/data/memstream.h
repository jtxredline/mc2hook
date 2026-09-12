#pragma once
#include <mc2hook\mc2hook.h>

class datMemStream
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
    int dword_2c;
    int dword_30;
    int dword_34;
    int dword_38;
    int dword_3c;
    int dword_40;
    int dword_44;
    int dword_48;
    int dword_4c;

public:
    static uint32_t GetCh(int stream);
    int Write(const void* data, unsigned int size);
};
