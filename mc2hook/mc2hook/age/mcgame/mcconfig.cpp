#include "mcconfig.h"
#include <age/core/output.h>
#include <age/types.h>
#include <age/globals.h>

declfield(mcConfig::g_SomePlayerCarModelTypeValue)(0x6C236C);
declfield(mcConfig::g_CurrentCar2)(0x6C3434);
declfield(mcConfig::g_SomeOtherCurrentCar)(0x6C3438);
declfield(mcConfig::g_CurrentCarColor2)(0x6C3458);

int mcConfig::LookupTOD(const char* todName)
{
    int index = 0;

    while (_stricmp(g_TODList[index], todName) != 0)
    {
        ++index;

        if (index >= TOD_COUNT)
        {
            Quitf("mcConfig::LookupTOD() failed");
            return 0;
        }
    }

    return index;
}

int mcConfig::LookupWeather(const char* weatherName)
{
    int index = 0;

    while (_stricmp(g_WeatherList[index], weatherName) != 0)
    {
        ++index;

        if (index >= WEATHER_COUNT)
        {
            Quitf("mcConfig::LookupWeather() failed");
            return 0;
        }
    }

    return index;
}
