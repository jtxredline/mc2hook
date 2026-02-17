#pragma once
#include <mc2hook\mc2hook.h>

class mcGameState {
public:
    void* m_Vtable;
    int m_CurrentState;
    bool m_IsPausedLocally;
    bool m_IsPaused;
    char field_A;
    char field_B;
    int dword_0c;
    int dword_10;
    int dword_14;
    int dword_18;
    int dword_1c;
    int dword_20;
    int dword_24;
    int dword_28;
    int dword_2c;
    int dword_30;
    int dword_34;
    int dword_38;
    int dword_3c;
    int dword_40;
    float m_Framerate;
    float dword_48;
    float dword_4c;
    float m_Realtime;
    int dword_54;
    int m_Framelock;
    char m_Swapearly;
    char field_5D;
    char field_5E;

public:
    void EnterState(int state);
    void EnterStateBoot();
    void EnterStateMovie();
    void EnterStateFrontend();
    void EnterStateRaceEditor();
    void EnterStateCarViewer();
    void EnterStateGame();
    void EnterStateReplay();
};
