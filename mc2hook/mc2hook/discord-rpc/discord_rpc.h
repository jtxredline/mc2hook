#pragma once
#include <string>

enum class GameState
{
    Boot,
    MainMenu,
    Loading,
    Race,
    RaceEditor,
};

void Discord_Init();
void Discord_Shutdown();
void Discord_RunCallbacks();
void Discord_UpdateForState(GameState state);

const char* GetCleanRaceName(const char* devName);
