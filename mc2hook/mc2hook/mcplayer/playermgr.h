#pragma once
#include <mc2hook\mc2hook.h>

class mcPlayer;

class mcPlayerManager
{
public:
    int m_Vtable;
    int dword_04;
    mcPlayer* m_Players[8];
    int m_NumPlayers;
    int m_PlayerStates[8];
    int m_PlayerRefs[8];

public:
    static hook::Type<mcPlayerManager*> Instance;

public:
    static mcPlayer* GetPlayer(int idx);
};
