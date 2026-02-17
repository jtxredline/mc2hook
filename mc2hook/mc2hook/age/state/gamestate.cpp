#include "gamestate.h"
#include <mc2hook\mc2hook.h>
#include <age/core/output.h>
#include <discord-rpc/discord_rpc.h>

void mcGameState::EnterState(int state)
{
    switch (state)
    {
    case 1:
        mcGameState::EnterStateBoot();
        break;
    case 2:
        mcGameState::EnterStateGame();
        Discord_UpdateForState(GameState::Loading);
        Discord_UpdateForState(GameState::Race);
        break;
    case 3:
        mcGameState::EnterStateReplay();
        break;
    case 4:
        mcGameState::EnterStateMovie();
        break;
    case 5:
        mcGameState::EnterStateFrontend();
        Discord_UpdateForState(GameState::MainMenu);
        break;
    case 6:
        mcGameState::EnterStateRaceEditor();
        Discord_UpdateForState(GameState::RaceEditor);
        break;
    case 7:
        mcGameState::EnterStateCarViewer();
        break;
    case 8:
        hook::Thunk<0x402AC0>::Call<int>(this);
        break;
    default:
        Quitf("mcGameState::EnterState - invalid State");
        break;
    }
    m_CurrentState = state;
    m_IsPaused = 0;
    m_IsPausedLocally = 0;
    this->field_A = 0;
}

void mcGameState::EnterStateBoot()
{
    hook::Thunk<0x403840>::Call<void>(this);
}
void mcGameState::EnterStateMovie()
{
    hook::Thunk<0x403860>::Call<void>(this);
}
void mcGameState::EnterStateFrontend()
{
    hook::Thunk<0x4038E0>::Call<void>(this);
}
void mcGameState::EnterStateRaceEditor()
{
    hook::Thunk<0x403AE0>::Call<void>(this);
}
void mcGameState::EnterStateCarViewer()
{
    hook::Thunk<0x403B30>::Call<void>(this);
}
void mcGameState::EnterStateGame()
{
    hook::Thunk<0x4047A0>::Call<void>(this);
}
void mcGameState::EnterStateReplay()
{
    hook::Thunk<0x4049E0>::Call<void>(this);
}
