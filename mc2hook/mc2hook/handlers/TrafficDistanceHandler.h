#pragma once
#include <mc2hook\mc2hook.h>

class TrafficDistanceHandler {
public:
    static void Install();

private:
    static float sm_TrafficDistanceMultiplier;
};
