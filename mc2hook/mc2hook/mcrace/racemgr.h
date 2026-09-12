#pragma once
#include <mc2hook\mc2hook.h>

class mcRaceBase;

class mcRaceManager
{
public:
    int dword_00;
    mcRaceBase* m_CurrentRace;
    int dword_08;
    int dword_0C;

public:
    static hook::Type<mcRaceManager*> Instance;
};
