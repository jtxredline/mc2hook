#include "TwoWheelHandler.h"
#include <age/data/timemgr.h>
#include <age/data/args.h>

#include <age/core/output.h>

static float lean = 0.0f;
static float leanDamp = 0.0f;
static float leanCached = 0.0f;
static float leanDampCached = 0.0f;
static float physicsFixesBaselineFPS = 0.0f;
static bool valuesCached = false;

void TwoWheelHandler::Update()
{
    hook::Thunk<0x4DC190>::Call<void>(this); // call original

    if (valuesCached == false)
    {
        leanCached = *getPtr<float>(this, 0x58);
        leanDampCached = *getPtr<float>(this, 0x60);
    }

    lean = leanCached * datTimeManager::GetSeconds() * physicsFixesBaselineFPS;
    leanDamp = leanDampCached * datTimeManager::GetSeconds() * physicsFixesBaselineFPS;

    Printf("LEAN: %f\r", lean);

    *getPtr<float>(this, 0x58) = lean;
    //*getPtr<float>(this, 0x60) = leanDamp;
}

void TwoWheelHandler::Install()
{
    physicsFixesBaselineFPS = HookConfig::GetFloat("Physics", "PhysicsFixesBaselineFPS", 30.0f);

    // Cap physics at 30fps as a lowest possible value
    if (physicsFixesBaselineFPS < 30.0f)
    {
        physicsFixesBaselineFPS = 30.0f;
    }

    InstallVTableHook("vehGyro Update", &Update, { 0x64556C });
}
