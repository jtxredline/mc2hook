#pragma once
#include <mc2hook\mc2hook.h>

class mcRaceState {
public:
    void* m_Vtable;
    int m_CurrentState;
    int dword_08;
    int dword_0C;
    float m_SomeDelta;
    int dword_14;
    float m_BeforeCountdownLength;
    float m_CountdownLength;
    int dword_20;
    int dword_24;
    int dword_28;
    int dword_2C;
    int dword_30;

public:
    static hook::Type<mcRaceState*> Instance;
    static hook::Type<mcRaceState*> InstanceCopy;
};
