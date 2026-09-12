#pragma once
#include <mc2hook/mc2hook.h>
#include <vector/vector3.h>

class phBound;

class phArchetype // TODO: Expand on this
{
public:
    int dword_00;
    int dword_04;
    phBound* m_Bound;
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
    float dword_3C;

public:
    static hook::Type<float> dword_674060; // 0.3f

    float sub_47B9D0();
    void SetTypeFlag(uint mask, bool value);
    void SetMass(float mass);
    void SetAngInertia(const Vector3& angInertia);
};
