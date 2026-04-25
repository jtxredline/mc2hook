#pragma once
#include <mc2hook\mc2hook.h>

class datTimeManager
{
public:
    static hook::Type<float> Seconds;
    static hook::Type<float> ActualSeconds;
    static hook::Type<float> FPS;
    static hook::Type<float> InvSeconds;
    static hook::Type<float> ElapsedTime;
    static hook::Type<float> PrevElapsedTime;
    static hook::Type<float> UnwarpedSeconds;

public:
    static float GetSeconds();
    static float GetActualSeconds();
    static float GetFPS();
    static float GetInvSeconds();
    static float GetElapsedTime();
    static float GetPrevElapsedTime();
    static float GetUnwarpedSeconds();

    static float PhysicsBaselineFPS;   // Read once from ini, min 30
    static float PhysicsSecondsScale;  // Cached: Seconds * PhysicsBaselineFPS, updated every frame

    static void InitPhysicsBaselineFPS();
    void Update();
};
