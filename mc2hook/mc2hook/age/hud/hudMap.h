#pragma once
#include <mc2hook\mc2hook.h>
//#include <age/vector/matrix44.h>

class Matrix44;

class hudMap
{
public:
    static hook::Type<float> dword_85B2A4;
    static hook::Type<float> dword_85B2B0;

    static hook::Type<int> dword_85B2AC; // Some ptr
    static hook::Type<int> dword_85B2C0;

public:
    //void sub_4A6230(void* a1, int a2, const Matrix44* mtx, float alpha);
    void sub_4A93C0(); // Some main function
};
