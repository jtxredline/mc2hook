#pragma once
#include <mc2hook\mc2hook.h>
#include <vector/vector3.h>

class vehEntity;

struct aiOpponentDesc
{
    uint8_t m_CarIndex;
    uint8_t field_01;
    uint8_t field_02;
    uint8_t field_03;
    uint8_t field_04;
    uint8_t field_05;
    uint8_t field_06;
    uint8_t field_07;
    uint8_t field_08;
    uint8_t field_09;
    bool m_IsCompetitor;
    uint8_t field_0B;
    float m_MaxThrottle;
    char buffer[108];
};

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
    int m_Team;
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
    char buffer[940];

public:
    aiOpponent()  { hook::Thunk<0x4B0030>::Call<void>(this); }
    ~aiOpponent() { hook::Thunk<0x4AFB10>::Call<void>(this); }

public:
    bool Init(int index, const char* carName, const aiOpponentDesc* desc, int a5, bool a6);
};

static_assert(sizeof(aiOpponent) == 0x42C, "aiOpponent size mismatch");
