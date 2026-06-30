#pragma once
#include <mc2hook\mc2hook.h>
#include <age/vector/vector3.h>

class vehEntity;

class aiOpponent
{
public:
    int dword_00;
    int dword_04;
    int dword_08;
    int dword_0C;
    int dword_10;
    int dword_14;
    int dword_18;
    vehEntity* m_Entity;
    Vector3 m_StartPos;
    float m_StartDir;
    int dword_30;
    int dword_34;
    int dword_38;
    int dword_3C;
    int dword_40;
    int dword_44;
    int dword_48;
    int dword_4C;
    int dword_50;
    void* m_Brain;
    void* m_Behavior;
    int dword_5C;
    int dword_60;
    int dword_64;
    float m_CarRadius;
    float m_CarRadiusSq;
    float m_HalfWidth;
    float m_HalfWidthSq;
    int dword_78;
    int dword_7C;
};

// Size: 0x42C ?