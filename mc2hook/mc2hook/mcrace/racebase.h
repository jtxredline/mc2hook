#pragma once
#include <mc2hook\mc2hook.h>

struct aiOpponentDesc;

class mcRaceBase
{
public:
    int m_Vtable;
    int dword_04;
    int dword_08;
    int m_NumOpponents;
    int dword_10;
    int dword_14;
    int dword_18;
    aiOpponentDesc* m_OpponentDescs;

public:
    void Destructor();
};
