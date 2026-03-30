#include "mcconfig.h"
#include <age/core/output.h>
#include <age/types.h>
#include <age/globals.h>

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
