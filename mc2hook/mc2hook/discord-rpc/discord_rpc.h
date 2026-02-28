#pragma once
#include <string>

enum class RPCState
{
    Boot,
    Frontend,
    Loading,
    Race,
    RaceEditor,
    LobbyOnline,
    LobbyLAN,
    Host,
};

void Discord_Init();
void Discord_Shutdown();
void Discord_RunCallbacks();
void Discord_UpdateForState(RPCState state);

const char* GetCleanRaceName(const char* devName);
