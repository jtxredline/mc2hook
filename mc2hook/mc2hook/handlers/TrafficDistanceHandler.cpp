#include "TrafficDistanceHandler.h"

static constexpr float DEFAULT_SPAWN_BASE = 200.0f;
static constexpr float DEFAULT_CULL_DISTANCE = 25.0f;
float TrafficDistanceHandler::sm_TrafficDistanceMultiplier = 0.0f;


void TrafficDistanceHandler::Install() {
    sm_TrafficDistanceMultiplier = HookConfig::GetFloat("Traffic", "TrafficDistanceMultiplier", 1.0f);

    float spawnBase = DEFAULT_SPAWN_BASE * sm_TrafficDistanceMultiplier;
    float cullDistance = DEFAULT_CULL_DISTANCE * sm_TrafficDistanceMultiplier;

    mem::write<float>(0x646DC8, spawnBase);
    mem::write<float>(0x662C70, cullDistance);
}
