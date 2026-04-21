#include "BurnoutHandler.h"
#include <age/data/timemgr.h>
#include <age/data/args.h>

static float burnoutIncreaseSpeed = 0.0f;
static float burnoutDecreaseSpeed = 0.0f;
static float burnoutDamageAmount = 0.0f;

static float burnoutIncreaseSpeedCached = 0.0f;
static float burnoutDecreaseSpeedCached = 0.0f;
static float burnoutDamageAmountCached = 0.0f;

static float physicsFixesBaselineFPS = 0.0f;

static bool vehCarSimValuesCached = false;

void BurnoutHandler::vehCarSimUpdate()
{
    hook::Thunk<0x4D3290>::Call<void>(this); // Call original

    if (vehCarSimValuesCached == false)
    {   
        burnoutIncreaseSpeedCached = *getPtr<float>(this, 0x190);
        burnoutDecreaseSpeedCached = *getPtr<float>(this, 0x194);
        burnoutDamageAmountCached = *getPtr<float>(this, 0x198);
        vehCarSimValuesCached = true;
    }

    burnoutIncreaseSpeed = burnoutIncreaseSpeedCached * datTimeManager::GetSeconds() * physicsFixesBaselineFPS; // Make this a member, part of datTimeManager
    burnoutDecreaseSpeed = burnoutDecreaseSpeedCached * datTimeManager::GetSeconds() * physicsFixesBaselineFPS;
    burnoutDamageAmount = burnoutDamageAmountCached * datTimeManager::GetSeconds() * physicsFixesBaselineFPS;

    *getPtr<float>(this, 0x190) = burnoutIncreaseSpeed;
    *getPtr<float>(this, 0x194) = burnoutDecreaseSpeed;
    *getPtr<float>(this, 0x198) = burnoutDamageAmount;
}

void BurnoutHandler::Install()
{
    physicsFixesBaselineFPS = HookConfig::GetFloat("Physics", "PhysicsFixesBaselineFPS", 60.0f);
    float minPhysicsFixesBaselineFPS = 30.0f;

    // Cap physics fixes baseline fps
    if (physicsFixesBaselineFPS < minPhysicsFixesBaselineFPS)
    {
        physicsFixesBaselineFPS = minPhysicsFixesBaselineFPS;
    }

    InstallVTableHook("vehCarSim Some Update", &vehCarSimUpdate, { 0x644A6C });
}
