#pragma once
#include <mc2hook\mc2hook.h>

class mcConfig
{
public:
    // These seem to be responsible for the default values in the menu
    static int LookupTOD(const char* todName);
    static int LookupWeather(const char* weatherName);

    static hook::Type<int[8]> g_SomePlayerCarModelTypeValue; //
    static hook::Type<int> g_CurrentCar2; //
    static hook::Type<int> g_SomeOtherCurrentCar; //
    static hook::Type<int> g_CurrentCarColor2; //
};
