#include "SteeringSmootherHandler.h"
#include <age/data/timemgr.h>


static float steerValue = 0.0f;
static float steeringRate = 7.0f;
static float centeringFactor = 0.9f;
static float steeringLock = 1.0f;
float controllerSteerSpeed;

void SteeringSmootherHandler::Update()
{
    // Preserve original game input behavior
    hook::Thunk<0x46B330>::Call<void>(this);
    float lastRequestedSteering = *getPtr<float>(this, 0x1C); // Steering input

    // Time-scaled adjustment
    float steeringDelta = lastRequestedSteering * steeringRate * datTimeManager::GetSeconds();

    // Smooth steering
    steerValue = (steerValue + steeringDelta) * centeringFactor;
    steerValue = fminf(fmaxf(steerValue, -steeringLock), steeringLock);

    *getPtr<float>(this, 0x1C) = steerValue;
}

void SteeringSmootherHandler::Install()
{
    if (HookConfig::GetBool("SmoothSteer", "SmoothSteerEnable", false) && !HookConfig::GetBool("KinematicSteer", "KinematicSteerEnable", false)) {
        steeringRate = HookConfig::GetFloat("SmoothSteer", "SteeringRate", 7.0f);
        centeringFactor = HookConfig::GetFloat("SmoothSteer", "CenteringFactor", 0.9f);
        steeringLock = HookConfig::GetFloat("SmoothSteer", "SteeringLock", 1.0f);
        InstallVTableHook("Input Update", &Update, { 0x63D0C4 });
    }

    controllerSteerSpeed = HookConfig::GetFloat("Input", "ControllerSteerSpeed", 10.0f);
    mem::write(0x46B28E + 3, static_cast<float>(controllerSteerSpeed));
}
