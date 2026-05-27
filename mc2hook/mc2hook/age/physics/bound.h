#pragma once

class phSurface;

class phBound // TODO: Expand on this
{
public:
    int dword_00;
    int dword_04;
    int dword_08;
    int dword_0C;
    int dword_10;
    int dword_14;
    int dword_18;
    int dword_1C;
    int dword_20;
    int dword_24;
    int dword_28;
    int dword_2C;
    int dword_30;
    int dword_34;
    int dword_38;
    int dword_3C;

public:
    phSurface* sub_5A5550(int a2, int a3);
};
