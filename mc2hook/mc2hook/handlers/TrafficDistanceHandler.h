#pragma once
#include <mc2hook\mc2hook.h>

class TrafficDistanceHandler {
public:
    static void Install();

private:
    static void LoadConfig();
    static void PatchDistances();

    static float sm_SpawnDistanceMultiplier;
    static float sm_CullDistanceMultiplier;
};
