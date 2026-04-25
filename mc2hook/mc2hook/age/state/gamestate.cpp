#include "gamestate.h"
#include <age/core/output.h>
#include <discord-rpc/discord_rpc.h>
#include <age/types.h>
#include <age/data/timemgr.h>

declfield(mcGameState::Instance)(0x6C3890);

void mcGameState::InitTime()
{
    hook::Thunk<0x402E10>::Call<void>(this); // Call original

    // General init for now
    datTimeManager::InitPhysicsBaselineFPS();
}

void mcGameState::EnterState(int state)
{
    switch (state)
    {
    case GameState::Boot:
        mcGameState::EnterStateBoot();
        break;
    case GameState::Game:
        mcGameState::EnterStateGame();
        Discord_UpdateForState(RPCState::Loading); // RPC refresh workaround
        Discord_UpdateForState(RPCState::Race);
        break;
    case GameState::Replay:
        mcGameState::EnterStateReplay();
        break;
    case GameState::Movie:
        mcGameState::EnterStateMovie();
        break;
    case GameState::Frontend:
        mcGameState::EnterStateFrontend();
        Discord_UpdateForState(RPCState::Frontend);
        break;
    case GameState::RaceEditor:
        mcGameState::EnterStateRaceEditor();
        Discord_UpdateForState(RPCState::RaceEditor);
        break;
    case GameState::CarViewer:
        mcGameState::EnterStateCarViewer();
        break;
    case GameState::Quit:
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
