#pragma once

class phSurface;

class phBound
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
    int dword_50;
    int dword_54;
    int dword_58;
    int dword_5c;
    int dword_60;
    int dword_64;
    int dword_68;
    int dword_6c;
    int dword_70;
    int dword_74;
    int dword_78;
    int dword_7c;
    int dword_80;
    int dword_84;
    int dword_88;
    int dword_8c;

public:
    phSurface* sub_5A5550(int a2, int a3);
    static phBound* Load(const char* boundName); // vehBound
};
