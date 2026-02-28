#pragma once
#include <mc2hook\mc2hook.h>

class mcRaceConfig {
public:
    static hook::Type<int> g_CurrentCity;
    static hook::Type<int> g_CurrentTOD;
    static hook::Type<int> g_CurrentWeather;
    static hook::Type<int> g_CurrentRaceType;
    static hook::Type<int> g_NumPlayerSlots;
    static hook::Type<char[256]> g_RaceName;
    static hook::Type<int> g_CurrentCar;
    static hook::Type<int> g_CurrentCarColor;
};
