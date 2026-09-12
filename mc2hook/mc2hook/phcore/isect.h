#pragma once

class Vector3;
class phArchetype;

class phIntersection // TODO: Needs checking
{
public:
    int dword_00;
    int dword_04;//phArchetype* m_Archetype;
    int m_LevelIndex;
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
    Vector3* sub_58FDD0(Vector3* a1, Vector3* a2); //(int a1, Vector3* a2);
};
