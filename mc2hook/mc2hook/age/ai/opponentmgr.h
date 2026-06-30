#pragma once
#include <mc2hook\mc2hook.h>

class aiOpponentManager
{
public:
    int m_NumOpponents;
    int m_NumCompetitors;
    void* m_Opponents;
    int m_CurrentOpponent;

public:
    static hook::Type<aiOpponentManager*> Instance;
};
