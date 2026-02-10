#include "TrafficDistanceHandler.h"

static constexpr float DEFAULT_SPAWN_BASE = 200.0f;
static constexpr float DEFAULT_CULL_DISTANCE = 25.0f;

float TrafficDistanceHandler::sm_SpawnDistanceMultiplier = 0.0f;
float TrafficDistanceHandler::sm_CullDistanceMultiplier = 0.0f;

void TrafficDistanceHandler::LoadConfig() {
    sm_SpawnDistanceMultiplier = HookConfig::GetFloat("TrafficDistance", "SpawnDistanceMultiplier", 2.0f);
    sm_CullDistanceMultiplier = HookConfig::GetFloat("TrafficDistance", "CullDistanceMultiplier", 2.0f);
}

void TrafficDistanceHandler::PatchDistances() {
    float spawnBase = DEFAULT_SPAWN_BASE * sm_SpawnDistanceMultiplier;
    float cullDistance = DEFAULT_CULL_DISTANCE * sm_CullDistanceMultiplier;

    mem::write<float>(0x646DC8, spawnBase);
    mem::write<float>(0x662C70, cullDistance);
}

void TrafficDistanceHandler::Install() {
    if (!HookConfig::GetBool("Traffic", "TrafficDistanceEnable", false))
        return;

    LoadConfig();
    PatchDistances();
}
