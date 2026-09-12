#include "timemgr.h"
#include <discord-rpc/discord_rpc.h>

declfield(datTimeManager::Seconds)(0x6797CC);
declfield(datTimeManager::InvSeconds)(0x6797D4);
declfield(datTimeManager::ActualSeconds)(0x6797D0);
declfield(datTimeManager::ElapsedTime)(0x8602E0);
declfield(datTimeManager::PrevElapsedTime)(0x8602D8);
declfield(datTimeManager::FPS)(0x6797D8);
declfield(datTimeManager::UnwarpedSeconds)(0x6797F4);

bool datTimeManager::EnableFPSFixes = true;
bool datTimeManager::SpeedrunMode = false;
float datTimeManager::PhysicsBaselineFPS = 60.0f;
float datTimeManager::PhysicsSecondsScale = 1.0f;

void datTimeManager::Init()
{
    PhysicsBaselineFPS = HookConfig::GetFloat("Physics", "PhysicsFixesBaselineFPS", 60.0f);
    
    // Cap physics fixes baseline fps
    float minPhysicsFixesBaselineFPS = 30.0f;
    if (PhysicsBaselineFPS < minPhysicsFixesBaselineFPS)
        PhysicsBaselineFPS = minPhysicsFixesBaselineFPS;

    EnableFPSFixes = HookConfig::GetBool("General", "EnableFPSFixes", true);
    SpeedrunMode = HookConfig::GetBool("General", "SpeedrunMode", false);
}

float datTimeManager::GetSeconds()
{
    return datTimeManager::Seconds.get();
}

float datTimeManager::GetInvSeconds()
{
    return datTimeManager::InvSeconds.get();
}

float datTimeManager::GetActualSeconds()
{
    return datTimeManager::ActualSeconds.get();
}

float datTimeManager::GetElapsedTime()
{
    return datTimeManager::ElapsedTime.get();
}

float datTimeManager::GetPrevElapsedTime()
{
    return datTimeManager::PrevElapsedTime.get();
}

float datTimeManager::GetFPS()
{
    return datTimeManager::FPS.get();
}

float datTimeManager::GetUnwarpedSeconds()
{
    return datTimeManager::UnwarpedSeconds.get();
}

void datTimeManager::Update()
{
    hook::Thunk<0x613FC0>::Call<void>(this); // Call original

    // Update physics FPS scaler
    PhysicsSecondsScale = EnableFPSFixes ? Seconds* PhysicsBaselineFPS : 1.0f;

    Discord_RunCallbacks();
}
